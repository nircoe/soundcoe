#include <soundcoe/resources/resource_manager.hpp>
#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <logcoe.hpp>

namespace soundcoe
{
    ResourceManager::ResourceManager() : m_initialized(false), m_maxSources(0), m_sourcePool(),
                                            m_freeSourceIndices(), m_bufferCache(), m_maxCacheSize(0), 
                                            m_currentCacheSize(0), m_loadedDirectories() { }

    ResourceManager::~ResourceManager() { shutdown(); }

    ResourceManager &ResourceManager::getInstance()
    {
        static ResourceManager instance;
        return instance;
    }

    void ResourceManager::initialize(const std::string &audioRootDirectory, size_t maxSources, size_t maxCacheSizeMB)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_audioRootDirectory = audioRootDirectory;
        m_maxSources = maxSources;
        m_maxCacheSize = maxCacheSizeMB;

        createSourcePool();
        m_initialized = true;
    }

    void ResourceManager::shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
    }

    bool ResourceManager::isInitialized() const 
    { 
        std::lock_guard<std::mutex> lock(m_mutex); 

        return m_initialized; 
    }

    bool ResourceManager::preloadDirectory(const std::string &subdirectory)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::filesystem::path fullPath = m_audioRootDirectory / subdirectory;
        if (!std::filesystem::exists(fullPath) || !std::filesystem::is_directory(fullPath))
        {
            logcoe::warning("Not a directory: \"" + subdirectory + "\"");
            return false;
        }

        if(isDirectoryLoaded(subdirectory))
        {
            logcoe::warning("Directory is already loaded: \"" + subdirectory + "\"");
            return true;
        }

        // loading logic - missing!

        m_loadedDirectories.push_back(subdirectory);
    }

    bool ResourceManager::unloadDirectory(const std::string &subdirectory)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!isDirectoryLoaded(subdirectory))
        {
            logcoe::warning("Directory is not loaded: \"" + subdirectory + "\"");
            return true;
        }

        // unload logic - missing!

        m_loadedDirectories.erase(std::remove(m_loadedDirectories.begin(), m_loadedDirectories.end(), subdirectory),
                                  m_loadedDirectories.end());
    }

    bool ResourceManager::preloadFile(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::filesystem::path fullPath = m_audioRootDirectory / filename;
        if (!std::filesystem::exists(fullPath) || !std::filesystem::is_regular_file(fullPath))
        {
            logcoe::warning("Not a File: \"" + filename + "\"");
            return false;
        }

        // load logic -- missing!
    }

    bool ResourceManager::unloadFile(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::filesystem::path fullPath = m_audioRootDirectory / filename;
        if (!std::filesystem::exists(fullPath) || !std::filesystem::is_regular_file(fullPath))
        {
            logcoe::warning("Not a File: \"" + filename + "\"");
            return true;
        }

        // unload logic -- missing!
    }

    bool ResourceManager::acquireSource(SoundSource *&source, SoundPriority priority)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

    }

    bool ResourceManager::releaseSource(SoundSource *source)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

    }

    bool ResourceManager::getBuffer(const std::string &filename, SoundBuffer *&buffer)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

    }

    void ResourceManager::releaseBuffer(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

    }

    size_t ResourceManager::getActiveSourceCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

    }

    size_t ResourceManager::getTotalSourceCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

    }

    size_t ResourceManager::getCachedBufferCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

    }

    size_t ResourceManager::getCacheSizeBytes() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

    }

    std::vector<std::filesystem::path> ResourceManager::getLoadedDirectories() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_loadedDirectories;
    }

    bool ResourceManager::isDirectoryLoaded(const std::string &subdirectory) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = std::find(m_loadedDirectories.begin(), m_loadedDirectories.end(), subdirectory);

        return it != m_loadedDirectories.end();
    }

    size_t ResourceManager::cleanupUnusedBuffers()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        size_t removed = 0;
        for (auto it = m_bufferCache.begin(); it != m_bufferCache.end();)
        {
            if(it->second.m_referenceCount != 0) 
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

    }

    void ResourceManager::destroySourcePool()
    {
        m_sourcePool.clear();
    }

    bool ResourceManager::findSourceToReplace(SoundPriority newPriority, size_t &replaceIndex)
    {
        if(m_sourcePool.empty())
        {
            logcoe::warning("findSourceToReplace called but m_sourcePool is empty");
            return false;
        }

        if(!m_freeSourceIndices.empty())
        {
            logcoe::warning("findSourceToReplace called but there are free sources available");
            return false;
        }

        auto minPrioritySource = std::min_element(m_sourcePool.begin(), m_sourcePool.end(),
                                                  [](const auto &a, const auto &b)
                                                  {
                                                      if (a.m_priority != b.m_priority)
                                                          return a.m_priority < b.m_priority;
                                                      return a.m_allocatedTime < b.m_allocatedTime;
                                                  });

        if(minPrioritySource->m_priority > newPriority) 
        {
            logcoe::debug("findSourceToReplace called but there is no lower priority source to replace with");
            return false;
        }

        replaceIndex = std::distance(m_sourcePool.begin(), minPrioritySource);
        logcoe::info("findSourceToReplace finished successfully");
        return true;
    }

    void ResourceManager::updateSourceAllocation(size_t index, SoundPriority priority)
    {
        m_sourcePool[index].m_priority = priority;
        m_sourcePool[index].m_allocatedTime = std::chrono::steady_clock::now();
        m_sourcePool[index].m_active = true;
    }

    void ResourceManager::freeOldestBuffer()
    {
        while(m_currentCacheSize > m_maxCacheSize && !m_bufferCache.empty()) 
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
                    allocation.m_source->stop();
                    allocation.m_source->detachBuffer();
                }
            }

            m_currentCacheSize -= oldest->second.m_buffer->getSize();
            m_bufferCache.erase(oldest);
        }
    }

    std::string ResourceManager::normalizeFilePath(const std::string &filename) const
    {
        return std::filesystem::path(filename).lexically_normal().string();
    }

    bool ResourceManager::scanDirectoryForAudioFiles(const std::filesystem::path &subdirectory, 
                                                        std::vector<std::filesystem::path> &audioFiles)
    {
        auto directoryFullPath = m_audioRootDirectory / subdirectory;
        bool foundAudioFile = false;
        for(const auto& entry : std::filesystem::recursive_directory_iterator(directoryFullPath))
        {
            if(checkIfAudioFile(entry.path()))
            {
                audioFiles.push_back(entry.path());
                foundAudioFile = true;
            }
        }

        std::string findText = foundAudioFile ? "We found audio files" : "We didn't find audio files";
        logcoe::info("scanDirectoryForAudioFiles finished, " + findText + " in Directory: " + directoryFullPath.string());
        return foundAudioFile; // this is why we return bool here? to say if we found a audio file?
    }

    bool ResourceManager::checkIfAudioFile(const std::filesystem::path &fullPath)
    {
        if(!std::filesystem::is_regular_file(fullPath)) return false;

        auto extension = fullPath.extension();
        return extension == ".wav" || extension == ".mp3" || extension == ".ogg";
    }
} // namespace soundcoe