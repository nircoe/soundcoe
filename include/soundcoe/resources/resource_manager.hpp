#pragma once

#include <soundcoe/core/types.hpp>
#include <soundcoe/resources/sound_buffer.hpp>
#include <soundcoe/resources/sound_source.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <deque>
#include <filesystem>

namespace soundcoe
{
    struct SourceAllocation
    {
        std::unique_ptr<SoundSource> m_source;
        SoundPriority m_priority;
        std::chrono::steady_clock::time_point m_allocatedTime;
        bool m_active;
    };

    struct BufferCacheEntry
    {
        std::unique_ptr<SoundBuffer> m_buffer;
        size_t m_referenceCount;
        std::chrono::steady_clock::time_point m_lastAccessed;
    };

    class ResourceManager
    {
        bool m_initialized;
        std::filesystem::path m_audioRootDirectory;
        size_t m_maxSources;
        mutable std::mutex m_mutex;

        std::vector<SourceAllocation> m_sourcePool;
        std::deque<size_t> m_freeSourceIndices;

        std::unordered_map<std::string, BufferCacheEntry> m_bufferCache;
        size_t m_maxCacheSize;
        size_t m_currentCacheSize;

        std::vector<std::filesystem::path> m_loadedDirectories;

        void createSourcePool();
        void destroySourcePool();
        bool findSourceToReplace(SoundPriority newPriority, size_t &replaceIndex);
        void updateSourceAllocation(size_t index, SoundPriority priority);
        void freeOldestBuffer();
        std::string normalizeFilePath(const std::string &filename) const;
        bool scanDirectoryForAudioFiles(const std::filesystem::path &subdirectory, std::vector<std::filesystem::path> &audioFiles);
        bool checkIfAudioFile(const std::filesystem::path &fullPath);

    public:
        static ResourceManager &getInstance();
        void initialize(const std::string &audioRootDirectory, size_t maxSources = 32, size_t maxCacheSizeMB = 64);
        void shutdown();
        bool isInitialized() const;

        bool preloadDirectory(const std::string &subdirectory);
        bool unloadDirectory(const std::string &subdirectory);
        bool preloadFile(const std::string &filename);
        bool unloadFile(const std::string &filename);

        bool acquireSource(SoundSource *&source, SoundPriority priority = SoundPriority::Medium);
        bool releaseSource(SoundSource *source);
        bool getBuffer(const std::string &filename, SoundBuffer *&buffer);
        void releaseBuffer(const std::string &filename);

        size_t getActiveSourceCount() const;
        size_t getTotalSourceCount() const;
        size_t getCachedBufferCount() const;
        size_t getCacheSizeBytes() const;
        std::vector<std::filesystem::path> getLoadedDirectories() const;
        bool isDirectoryLoaded(const std::string &subdirectory) const;
        size_t cleanupUnusedBuffers();
    };
} // namespace soundcoe