#include <soundcoe/resources/resource_manager.hpp>
#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <logcoe.hpp>
#include <algorithm>

namespace soundcoe
{
    namespace detail
    {
        ResourceManager::ResourceManager() : m_audioContext(), m_audioRootDirectory(), m_sourcePool(),
                                             m_freeSourceIndices(), m_bufferCache(), m_loadedDirectories() {}

        ResourceManager::~ResourceManager() { shutdown(); }

        void ResourceManager::initialize(const std::string &audioRootDirectory, size_t maxSources,
                                        size_t maxCacheSizeMB)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_initialized)
            {
                logcoe::info("ResourceManager is already initialized");
                return;
            }

            if (audioRootDirectory.empty())
            {
                logcoe::warning("Audio root directory cannot be empty - specify a valid directory path");
                return;
            }

            m_audioContext.initialize();

            m_audioRootDirectory = normalizePath(audioRootDirectory);
            m_maxSources = maxSources;
            m_maxCacheSize = maxCacheSizeMB * 1024 * 1024;

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
            logcoe::info("ResourceManager initialized successfully");
        }

        void ResourceManager::shutdown()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (!m_initialized)
                return;

            m_sourcePool.clear();
            m_bufferCache.clear();
            m_loadedDirectories.clear();
            m_freeSourceIndices.clear();

            try { m_audioContext.shutdown(); }
            catch(const std::runtime_error &) { logcoe::warning("ResourceManager::shutdown: Failed to shutdown the AudioContext"); }

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
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return false;
            }

            if (subdirectory.empty())
            {
                logcoe::warning("Cannot preload empty subdirectory - this would load the entire audio root directory");
                return false;
            }

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
            if (scanDirectoryForFiles(subdirectory, audioFiles))
            {
                for (const auto &file : audioFiles)
                    preloadFileImpl(file);

                m_loadedDirectories.push_back(subdirectory);
                return true;
            }

            logcoe::warning("No audio files found in directory: " + subdirectory);
            return false;
        }

        bool ResourceManager::unloadDirectory(const std::string &subdirectory)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return false;
            }

            if (subdirectory.empty())
            {
                logcoe::warning("Subdirectory path cannot be empty - specify a valid directory path");
                return true;
            }

            if (!isDirectoryLoadedImpl(subdirectory))
            {
                logcoe::warning("Directory is not loaded: \"" + subdirectory + "\"");
                return true;
            }

            std::vector<std::filesystem::path> audioFiles;
            if (scanDirectoryForFiles(subdirectory, audioFiles))
                for (const auto &file : audioFiles)
                    unloadFileImpl(file);
            else
                logcoe::warning("No audio files found in directory: " + subdirectory);

            m_loadedDirectories.erase(std::remove(m_loadedDirectories.begin(), m_loadedDirectories.end(), subdirectory),
                                    m_loadedDirectories.end());
            return true;
        }

        std::optional<std::reference_wrapper<SoundSource>> ResourceManager::acquireSource(size_t &poolIndex, SoundPriority priority)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return std::nullopt;
            }

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

            poolIndex = index;
            return std::ref(*(entry.m_source));
        }

        std::optional<std::reference_wrapper<SoundBuffer>> ResourceManager::getBuffer(const std::string &filename)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return std::nullopt;
            }

            if (filename.empty())
            {
                logcoe::error("Filename cannot be empty - specify a valid audio file path");
                return std::nullopt;
            }
            std::filesystem::path foundPath = findFileInLoadedDirectories(filename);
            if(foundPath.empty())
            {
                logcoe::error("No such file in the loaded directories: " + filename);
                return std::nullopt;
            }

            std::string cacheKey = foundPath.lexically_normal().string();
            if ((m_bufferCache.find(cacheKey) == m_bufferCache.end()) && !preloadFileImpl(foundPath))
                return std::nullopt;

            auto &entry = m_bufferCache[cacheKey];
            ++entry.m_referenceCount;
            entry.m_lastAccessed = std::chrono::steady_clock::now();
            return std::ref(*(entry.m_buffer));
        }

        bool ResourceManager::releaseSource(std::reference_wrapper<SoundSource> source)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return false;
            }

            for (size_t i = 0; i < m_sourcePool.size(); ++i)
            {
                auto &allocation = m_sourcePool[i];
                if (!allocation.m_active || allocation.m_source->getSourceId() != source.get().getSourceId())
                    continue;

                try
                {
                    allocation.m_source->detachBuffer();
                }
                catch (const std::exception &e)
                {
                    logcoe::warning("Failed to detach Buffer: " + std::string(e.what()));
                }

                allocation.m_active = false;
                m_freeSourceIndices.push_back(i);

                return true;
            }

            logcoe::warning("This SoundSource is not acquired");
            return true;
        }

        bool ResourceManager::releaseBuffer(std::reference_wrapper<SoundBuffer> buffer)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return releaseBufferImpl(buffer.get().getFileName());
        }

        bool ResourceManager::releaseBuffer(const std::string &filename)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return releaseBufferImpl(filename);
        }

        size_t ResourceManager::getActiveSourceCount() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return 0;
            }

            return m_sourcePool.size() - m_freeSourceIndices.size();
        }

        size_t ResourceManager::getTotalSourceCount() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return 0;
            }

            return m_sourcePool.size();
        }

        size_t ResourceManager::getCachedBufferCount() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return 0;
            }

            return m_bufferCache.size();
        }

        size_t ResourceManager::getCacheSizeBytes() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return 0;
            }

            return m_currentCacheSize;
        }

        std::vector<std::filesystem::path> ResourceManager::getLoadedDirectories() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return {};
            }

            return m_loadedDirectories;
        }

        bool ResourceManager::isDirectoryLoaded(const std::string &subdirectory) const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return false;
            }

            if (subdirectory.empty())
            {
                logcoe::warning("Subdirectory cannot be empty - specify a valid directory path");
                return false;
            }

            return isDirectoryLoadedImpl(subdirectory);
        }

        size_t ResourceManager::cleanupUnusedBuffers()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return 0;
            }

            size_t removed = 0;
            for (auto it = m_bufferCache.begin(); it != m_bufferCache.end();)
            {
                if (it->second.m_referenceCount == 0)
                {
                    m_currentCacheSize -= it->second.m_buffer->getSize();
                    it = m_bufferCache.erase(it);
                    ++removed;
                }
                else
                    ++it;
            }

            return removed;
        }

        void ResourceManager::createSourcePool()
        {
            m_sourcePool.resize(m_maxSources);
            m_freeSourceIndices.clear();
            auto time = std::chrono::steady_clock::now();

            for (size_t index = 0; index < m_maxSources; ++index)
            {
                m_sourcePool[index].m_source = std::make_unique<SoundSource>();
                m_sourcePool[index].m_priority = SoundPriority::Medium;
                m_sourcePool[index].m_allocatedTime = time;
                m_sourcePool[index].m_active = false;
                m_freeSourceIndices.push_back(index);
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
                {
                    sourceToReplace->m_source->stop();
                    sourceToReplace->m_active = false;
                }
            }

            replaceIndex = std::distance(m_sourcePool.begin(), sourceToReplace);
            logcoe::info("findSourceToReplace finished successfully");
            return true;
        }

        void ResourceManager::freeBuffers()
        {
            while (m_currentCacheSize > m_maxCacheSize && !m_bufferCache.empty())
            {
                auto toFree = std::min_element(m_bufferCache.begin(), m_bufferCache.end(),
                                            [this](const auto &a, const auto &b)
                                            {
                                                if (a.second.m_referenceCount == 0 && b.second.m_referenceCount > 0)
                                                    return true;
                                                if (a.second.m_referenceCount > 0 && b.second.m_referenceCount == 0)
                                                    return false;

                                                auto aPriority = getHighestPriorityForBuffer(a.second.m_buffer->getBufferId());
                                                auto bPriority = getHighestPriorityForBuffer(b.second.m_buffer->getBufferId());
                                                if (aPriority != bPriority)
                                                    return aPriority < bPriority;

                                                return a.second.m_lastAccessed < b.second.m_lastAccessed;
                                            });

                auto oldestBufferId = toFree->second.m_buffer->getBufferId();
                for (size_t i = 0; i < m_sourcePool.size(); ++i)
                {
                    auto &allocation = m_sourcePool[i];
                    if (!allocation.m_active || allocation.m_source->getBufferId() != oldestBufferId)
                        continue;

                    try
                    {
                        allocation.m_source->detachBuffer();
                    }
                    catch (const std::exception &e)
                    {
                        logcoe::warning("Failed to detach Buffer: " + std::string(e.what()));
                    }

                    allocation.m_active = false;
                    m_freeSourceIndices.push_back(i);
                }

                m_currentCacheSize -= toFree->second.m_buffer->getSize();
                m_bufferCache.erase(toFree);
            }
        }

        std::filesystem::path ResourceManager::normalizePath(const std::string &path) const
        {
            return std::filesystem::path(path).lexically_normal();
        }

        bool ResourceManager::scanDirectoryForFiles(const std::filesystem::path &subdirectory,
                                                    std::vector<std::filesystem::path> &files)
        {
            auto directoryFullPath = m_audioRootDirectory / subdirectory.lexically_normal();
            bool foundFile = false;
            try
            {
                for (const auto &entry : std::filesystem::recursive_directory_iterator(directoryFullPath))
                {
                    try
                    {
                        if (entry.is_regular_file())
                        {
                            files.push_back(entry.path());
                            foundFile = true;
                        }
                    }
                    catch (const std::filesystem::filesystem_error &e)
                    {
                        logcoe::warning("std::filesystem exception for file: " + entry.path().string() + ": " + std::string(e.what()));
                    }
                }
            }
            catch (const std::exception &e)
            {
                logcoe::warning("Failed to scan directory: " + std::string(e.what()));
            }

            std::string findText = foundFile ? "We found files" : "We didn't find files";
            logcoe::info("scanDirectoryForFiles finished, " + findText + " in Directory: " + directoryFullPath.string());
            return foundFile;
        }

        bool ResourceManager::preloadFileImpl(const std::filesystem::path &filePath)
        {
            if (!filePath.is_absolute())
            {
                logcoe::error("File path is not absolute: " + filePath.string());
                return false;
            }

            try
            {
                if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath))
                {
                    logcoe::error("Not a File: \"" + filePath.string() + "\"");
                    return false;
                }
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                logcoe::error("std::filesystem exception: " + std::string(e.what()));
                return false;
            }

            std::string cacheKey = filePath.string();
            if (m_bufferCache.find(cacheKey) != m_bufferCache.end())
                return true;

            try
            {
                BufferCacheEntry entry;

                entry.m_buffer = std::make_unique<SoundBuffer>(cacheKey);
                entry.m_referenceCount = 0;
                entry.m_lastAccessed = std::chrono::steady_clock::now();

                m_currentCacheSize += entry.m_buffer->getSize();
                m_bufferCache[cacheKey] = std::move(entry);
            }
            catch (const std::exception &e)
            {
                logcoe::error("Failed to create SoundBuffer: " + std::string(e.what()));
                m_bufferCache.erase(cacheKey);
                return false;
            }

            if (m_currentCacheSize > m_maxCacheSize)
                freeBuffers();

            logcoe::info("preloadFile Successfully: \"" + cacheKey + "\"");
            return true;
        }

        bool ResourceManager::unloadFileImpl(const std::filesystem::path &filePath)
        {
            try
            {
                if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath))
                {
                    logcoe::warning("Not a File: \"" + filePath.string() + "\"");
                    return true;
                }
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                logcoe::error("std::filesystem exception: " + std::string(e.what()));
                return false;
            }

            std::string cacheKey = filePath.string();
            if (m_bufferCache.find(cacheKey) == m_bufferCache.end())
            {
                logcoe::warning("File is not loaded: \"" + cacheKey + "\"");
                return true;
            }

            auto &entry = m_bufferCache[cacheKey];
            if (entry.m_referenceCount > 0)
            {
                auto bufferId = entry.m_buffer->getBufferId();
                for (size_t i = 0; i < m_sourcePool.size(); ++i)
                {
                    auto &allocation = m_sourcePool[i];
                    if (!allocation.m_active || allocation.m_source->getBufferId() != bufferId)
                        continue;

                    try
                    {
                        allocation.m_source->detachBuffer();
                    }
                    catch (const std::exception &e)
                    {
                        logcoe::warning("Failed to detach Buffer: " + std::string(e.what()));
                    }

                    allocation.m_active = false;
                    m_freeSourceIndices.push_back(i);
                }
            }

            m_currentCacheSize -= entry.m_buffer->getSize();
            m_bufferCache.erase(cacheKey);
            return true;
        }

        bool ResourceManager::isDirectoryLoadedImpl(const std::string &subdirectory) const
        {
            auto it = std::find(m_loadedDirectories.begin(), m_loadedDirectories.end(), subdirectory);

            return it != m_loadedDirectories.end();
        }

        SoundPriority ResourceManager::getHighestPriorityForBuffer(ALuint bufferId) const
        {
            SoundPriority highest = SoundPriority::Low;
            for (const auto &allocation : m_sourcePool)
            {
                if (allocation.m_active && allocation.m_source->getBufferId() == bufferId &&
                    allocation.m_priority > highest)
                {
                    highest = allocation.m_priority;
                }
            }

            return highest;
        }

        bool ResourceManager::releaseBufferImpl(const std::string &filename)
        {
            if (!m_initialized)
            {
                logcoe::error("ResourceManager is not initialized");
                return false;
            }

            std::filesystem::path foundPath = findFileInLoadedDirectories(filename);
            if(foundPath.empty())
            {
                logcoe::warning("No such file found for release: " + filename);
                return true;
            }

            std::string cacheKey = foundPath.lexically_normal().string();
            if (m_bufferCache.find(cacheKey) == m_bufferCache.end())
            {
                logcoe::warning("Buffer is not loaded in cache: " + cacheKey);
                return true;
            }

            auto &entry = m_bufferCache[cacheKey];
            if (entry.m_referenceCount == 0)
            {
                logcoe::warning("Not a single Source is using this Buffer at the moment");
                return true;
            }

            --entry.m_referenceCount;
            return true;
        }

        std::filesystem::path ResourceManager::findFileInLoadedDirectories(const std::string &filename) const
        {
            auto loadedDirs = m_loadedDirectories;
            for(const auto &dir : loadedDirs)
            {
                std::filesystem::path candidatePath = (m_audioRootDirectory / dir / filename).lexically_normal();
                if (std::filesystem::exists(candidatePath) && std::filesystem::is_regular_file(candidatePath))
                {
                    return candidatePath;
                }
            }
            return std::filesystem::path();
        }

        std::optional<std::reference_wrapper<SourceAllocation>> ResourceManager::getSourceAllocation(size_t index)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (index < m_sourcePool.size())
                return std::ref(m_sourcePool[index]);
            return std::nullopt;
        }
    } // namespace detail
} // namespace soundcoe
