#include <soundcoe/resources/resource_manager.hpp>
#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <logcoe.hpp>
#include <algorithm>

namespace soundcoe
{
    ResourceManager::ResourceManager() : m_initialized(false), m_maxSources(0), m_sourcePool(),
                                         m_freeSourceIndices(), m_bufferCache(), m_maxCacheSize(0),
                                         m_currentCacheSize(0), m_loadedDirectories() {}

    ResourceManager::~ResourceManager() { shutdown(); }

    ResourceManager &ResourceManager::getInstance()
    {
        static ResourceManager instance;
        return instance;
    }

    void ResourceManager::initialize(const std::string &audioRootDirectory, size_t maxSources, size_t maxCacheSizeMB)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_audioRootDirectory = normalizePath(audioRootDirectory);
        m_maxSources = maxSources;
        m_maxCacheSize = maxCacheSizeMB;

        try
        {
            createSourcePool();
        }
        catch (const std::exception &e)
        {
            logcoe::error("Failed to create Source Pool: " + std::string(e.what()));
            m_sourcePool.clear();
            m_freeSourceIndices.clear();
            throw;
        }

        m_initialized = true;
    }

    void ResourceManager::shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_sourcePool.clear();
        m_bufferCache.clear();
        m_loadedDirectories.clear();
        m_freeSourceIndices.clear();

        m_currentCacheSize = 0;
        m_initialized = false;
    }

    bool ResourceManager::isInitialized() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_initialized;
    }

    bool ResourceManager::preloadDirectory(const std::string &subdirectory)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::filesystem::path fullPath = m_audioRootDirectory / normalizePath(subdirectory);
        if (!std::filesystem::exists(fullPath) || !std::filesystem::is_directory(fullPath))
        {
            logcoe::warning("Not a directory: \"" + subdirectory + "\"");
            return false;
        }

        if (isDirectoryLoadedImpl(subdirectory))
        {
            logcoe::warning("Directory is already loaded: \"" + subdirectory + "\"");
            return true;
        }

        std::vector<std::filesystem::path> audioFiles;
        if (scanDirectoryForAudioFiles(subdirectory, audioFiles))
        {
            for (auto &file : audioFiles)
                preloadFileImpl(file.string());

            m_loadedDirectories.push_back(subdirectory);
            return true;
        }

        logcoe::warning("No audio files found in directory: " + subdirectory);
        return false;
    }

    bool ResourceManager::unloadDirectory(const std::string &subdirectory)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!isDirectoryLoadedImpl(subdirectory))
        {
            logcoe::warning("Directory is not loaded: \"" + subdirectory + "\"");
            return true;
        }

        std::vector<std::filesystem::path> audioFiles;
        if (scanDirectoryForAudioFiles(subdirectory, audioFiles))
            for (auto &file : audioFiles)
                unloadFileImpl(file.string());
        else
            logcoe::warning("No audio files found in directory: " + subdirectory);

        m_loadedDirectories.erase(std::remove(m_loadedDirectories.begin(), m_loadedDirectories.end(), subdirectory),
                                  m_loadedDirectories.end());
        return true;
    }

    bool ResourceManager::preloadFile(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return preloadFileImpl(filename);
    }

    bool ResourceManager::unloadFile(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return unloadFileImpl(filename);
    }

    std::optional<std::reference_wrapper<SoundSource>> ResourceManager::acquireSource(SoundPriority priority)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        size_t index;
        if (m_freeSourceIndices.empty())
        {
            if (!findSourceToReplace(priority, index))
            {
                logcoe::error("Could not find a Source to replace");
                return std::nullopt;
            }
        }
        else
        {
            index = m_freeSourceIndices.front();
            m_freeSourceIndices.pop_front();
        }

        auto &entry = m_sourcePool[index];
        entry.m_priority = priority;
        entry.m_allocatedTime = std::chrono::steady_clock::now();
        entry.m_active = true;

        if (entry.m_source.get() == nullptr)
            return std::nullopt;

        return std::ref(*(entry.m_source));
    }

    std::optional<std::reference_wrapper<SoundBuffer>> ResourceManager::getBuffer(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string fullPath = (m_audioRootDirectory / normalizePath(filename)).string();
        if (m_bufferCache.find(fullPath) == m_bufferCache.end())
            if (!preloadFileImpl(filename))
                return std::nullopt;

        auto &entry = m_bufferCache[fullPath];
        ++entry.m_referenceCount;
        entry.m_lastAccessed = std::chrono::steady_clock::now();
        return std::ref(*(entry.m_buffer));
    }

    bool ResourceManager::releaseSource(std::reference_wrapper<SoundSource> source)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (size_t i = 0; i < m_sourcePool.size(); ++i)
        {
            if (m_sourcePool[i].m_source->getSourceId() != source.get().getSourceId())
                continue;

            try
            {
                m_sourcePool[i].m_source->detachBuffer();
            }
            catch (const std::exception &e)
            {
                logcoe::warning("Failed to detach Buffer: " + std::string(e.what()));
            }

            m_sourcePool[i].m_active = false;
            m_freeSourceIndices.push_back(i);

            return true;
        }

        logcoe::warning("This SoundSource is not acquired");
        return true;
    }

    bool ResourceManager::releaseBuffer(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string fullPath = (m_audioRootDirectory / normalizePath(filename)).string();
        if (m_bufferCache.find(fullPath) == m_bufferCache.end())
        {
            logcoe::warning("File is not loaded: \"" + fullPath + "\"");
            return true;
        }

        auto &entry = m_bufferCache[fullPath];

        if (entry.m_referenceCount == 0)
        {
            logcoe::warning("Not a single Source is using this Buffer at the moment");
            return true;
        }

        --entry.m_referenceCount;
        return true;
    }

    size_t ResourceManager::getActiveSourceCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_sourcePool.size() - m_freeSourceIndices.size();
    }

    size_t ResourceManager::getTotalSourceCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_sourcePool.size();
    }

    size_t ResourceManager::getCachedBufferCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_bufferCache.size();
    }

    size_t ResourceManager::getCacheSizeBytes() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_currentCacheSize;
    }

    std::vector<std::filesystem::path> ResourceManager::getLoadedDirectories() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_loadedDirectories;
    }

    bool ResourceManager::isDirectoryLoaded(const std::string &subdirectory) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return isDirectoryLoadedImpl(subdirectory);
    }

    size_t ResourceManager::cleanupUnusedBuffers()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        size_t removed = 0;
        for (auto it = m_bufferCache.begin(); it != m_bufferCache.end();)
        {
            if (it->second.m_referenceCount != 0)
            {
                ++it;
                continue;
            }

            m_currentCacheSize -= it->second.m_buffer->getSize();
            it = m_bufferCache.erase(it);
            ++removed;
        }

        return removed;
    }

    void ResourceManager::createSourcePool()
    {
        m_sourcePool.resize(m_maxSources);
        m_freeSourceIndices.resize(m_maxSources);
        auto time = std::chrono::steady_clock::now();

        for (size_t index = 0; index < m_maxSources; ++index)
        {
            m_sourcePool[index].m_source = std::make_unique<SoundSource>();
            m_sourcePool[index].m_priority = SoundPriority::Medium;
            m_sourcePool[index].m_allocatedTime = time;
            m_sourcePool[index].m_active = false;
            m_freeSourceIndices[index] = index;
        }
    }

    bool ResourceManager::findSourceToReplace(SoundPriority newPriority, size_t &replaceIndex)
    {
        if (m_sourcePool.empty())
        {
            logcoe::warning("findSourceToReplace called but m_sourcePool is empty");
            return false;
        }

        if (!m_freeSourceIndices.empty())
        {
            logcoe::warning("findSourceToReplace called but there are free sources available");
            return false;
        }

        auto sourceToReplace = std::find_if(m_sourcePool.begin(), m_sourcePool.end(),
                                            [](const auto &it)
                                            {
                                                return it.m_active && it.m_source->isStopped();
                                            });
        if (sourceToReplace == m_sourcePool.end())
        {
            sourceToReplace = std::min_element(m_sourcePool.begin(), m_sourcePool.end(),
                                               [](const auto &a, const auto &b)
                                               {
                                                   if (a.m_priority != b.m_priority)
                                                       return a.m_priority < b.m_priority;
                                                   return a.m_allocatedTime < b.m_allocatedTime;
                                               });

            if (sourceToReplace->m_priority > newPriority)
            {
                logcoe::debug("findSourceToReplace called but there is no lower priority source to replace with");
                return false;
            }

            if (sourceToReplace->m_active)
                sourceToReplace->m_source->stop();
        }

        replaceIndex = std::distance(m_sourcePool.begin(), sourceToReplace);
        logcoe::info("findSourceToReplace finished successfully");
        return true;
    }

    void ResourceManager::freeOldestBuffer()
    {
        while (m_currentCacheSize > m_maxCacheSize && !m_bufferCache.empty())
        {
            auto oldest = std::min_element(m_bufferCache.begin(), m_bufferCache.end(),
                                           [](const auto &a, const auto &b)
                                           {
                                               return a.second.m_lastAccessed < b.second.m_lastAccessed;
                                           });

            auto oldestBufferId = oldest->second.m_buffer->getBufferId();
            for (auto &allocation : m_sourcePool)
            {
                if (!allocation.m_active)
                    continue;

                if (allocation.m_source->getBufferId() == oldestBufferId)
                {
                    try
                    {
                        allocation.m_source->detachBuffer();
                    }
                    catch (const std::exception &e)
                    {
                        logcoe::warning("Failed to detach Buffer: " + std::string(e.what()));
                    }
                }
            }

            m_currentCacheSize -= oldest->second.m_buffer->getSize();
            m_bufferCache.erase(oldest);
        }
    }

    std::filesystem::path ResourceManager::normalizePath(const std::filesystem::path &path) const
    {
        return std::filesystem::path(path).lexically_normal().string();
    }

    bool ResourceManager::scanDirectoryForAudioFiles(const std::filesystem::path &subdirectory,
                                                     std::vector<std::filesystem::path> &audioFiles)
    {
        auto directoryFullPath = m_audioRootDirectory / normalizePath(subdirectory);
        bool foundAudioFile = false;
        try
        {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(directoryFullPath))
            {
                if (checkIfSupportedAudioFile(entry.path()))
                {
                    audioFiles.push_back(entry.path());
                    foundAudioFile = true;
                }
            }
        }
        catch (const std::exception &e)
        {
            logcoe::warning("Failed to scan directory: " + std::string(e.what()));
        }

        std::string findText = foundAudioFile ? "We found audio files" : "We didn't find audio files";
        logcoe::info("scanDirectoryForAudioFiles finished, " + findText + " in Directory: " + directoryFullPath.string());
        return foundAudioFile;
    }

    bool ResourceManager::checkIfSupportedAudioFile(const std::filesystem::path &fullPath)
    {
        try
        {
            if (!std::filesystem::is_regular_file(fullPath))
                return false;

            auto extension = fullPath.extension();
            return extension == ".wav" || extension == ".mp3" || extension == ".ogg";
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            logcoe::warning("std::filesystem exception: " + std::string(e.what()));
            return false;
        }
    }

    bool ResourceManager::preloadFileImpl(const std::string &filename)
    {
        std::filesystem::path fullPath;
        if (std::filesystem::path(filename).is_absolute())
            fullPath = filename;
        else
            fullPath = m_audioRootDirectory / normalizePath(filename);

        try
        {
            if (!std::filesystem::exists(fullPath) || !std::filesystem::is_regular_file(fullPath))
            {
                logcoe::error("Not a File: \"" + fullPath.string() + "\"");
                return false;
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            logcoe::error("std::filesystem exception: " + std::string(e.what()));
            return false;
        }

        if (!checkIfSupportedAudioFile(fullPath))
        {
            logcoe::error("Not a Supported Audio File (.wav, .mp3, .ogg only): \"" + fullPath.string() + "\"");
            return false;
        }

        std::string fullPathStr = fullPath.string();
        if (m_bufferCache.find(fullPathStr) != m_bufferCache.end())
            return true;

        try
        {
            BufferCacheEntry entry;

            entry.m_buffer = std::make_unique<SoundBuffer>(fullPathStr);
            entry.m_referenceCount = 0;
            entry.m_lastAccessed = std::chrono::steady_clock::now();

            m_currentCacheSize += entry.m_buffer->getSize();
            m_bufferCache[fullPathStr] = std::move(entry);
        }
        catch (const std::exception &e)
        {
            logcoe::error("Failed to create SoundBuffer: " + std::string(e.what()));
            m_bufferCache.erase(fullPathStr);
            return false;
        }

        if (m_currentCacheSize > m_maxCacheSize)
            freeOldestBuffer();

        logcoe::info("preloadFile Successfully: \"" + fullPathStr + "\"");
        return true;
    }

    bool ResourceManager::unloadFileImpl(const std::string &filename)
    {
        std::filesystem::path fullPath;
        if (std::filesystem::path(filename).is_absolute())
            fullPath = filename;
        else
            fullPath = m_audioRootDirectory / normalizePath(filename);

        try
        {
            if (!std::filesystem::exists(fullPath) || !std::filesystem::is_regular_file(fullPath))
            {
                logcoe::warning("Not a File: \"" + filename + "\"");
                return true;
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            logcoe::error("std::filesystem exception: " + std::string(e.what()));
            return false; // we don't know if filename is a audio file so it may be a failure of unloadFileImpl
        }

        if (m_bufferCache.find(fullPath.string()) == m_bufferCache.end())
        {
            logcoe::warning("File is not loaded: \"" + fullPath.string() + "\"");
            return true;
        }

        auto &entry = m_bufferCache[fullPath.string()];
        if (entry.m_referenceCount > 0)
        {
            auto bufferId = entry.m_buffer->getBufferId();
            for (auto &it : m_sourcePool)
            {
                if (it.m_active && it.m_source->getBufferId() == bufferId)
                {
                    try
                    {
                        it.m_source->detachBuffer();
                    }
                    catch (const std::exception &e)
                    {
                        logcoe::warning("Failed to detach Buffer: " + std::string(e.what()));
                    }
                }
            }
        }

        m_currentCacheSize -= entry.m_buffer->getSize();
        m_bufferCache.erase(fullPath.string());
        return true;
    }

    bool ResourceManager::isDirectoryLoadedImpl(const std::string &subdirectory) const
    {
        auto it = std::find(m_loadedDirectories.begin(), m_loadedDirectories.end(), subdirectory);

        return it != m_loadedDirectories.end();
    }
} // namespace soundcoe
