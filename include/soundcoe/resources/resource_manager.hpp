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
#include <optional>
#include <functional>

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
        bool findSourceToReplace(SoundPriority newPriority, size_t &replaceIndex);
        void freeOldestBuffer();
        std::filesystem::path normalizePath(const std::filesystem::path &path) const;
        bool scanDirectoryForAudioFiles(const std::filesystem::path &subdirectory, std::vector<std::filesystem::path> &audioFiles);
        bool checkIfSupportedAudioFile(const std::filesystem::path &fullPath);
        bool preloadFileImpl(const std::string &filename);
        bool unloadFileImpl(const std::string &filename);
        bool isDirectoryLoadedImpl(const std::string &subdirectory) const;

    public:
        static ResourceManager &getInstance();
        void initialize(const std::string &audioRootDirectory, size_t maxSources = 32, size_t maxCacheSizeMB = 64);
        void shutdown();
        bool isInitialized() const;

        bool preloadDirectory(const std::string &subdirectory);
        bool unloadDirectory(const std::string &subdirectory);
        bool preloadFile(const std::string &filename);
        bool unloadFile(const std::string &filename);

        std::optional<std::reference_wrapper<SoundSource>> acquireSource(SoundPriority priority = SoundPriority::Medium);
        std::optional<std::reference_wrapper<SoundBuffer>> getBuffer(const std::string &filename);
        bool releaseSource(std::reference_wrapper<SoundSource> source);
        bool releaseBuffer(const std::string &filename);

        size_t getActiveSourceCount() const;
        size_t getTotalSourceCount() const;
        size_t getCachedBufferCount() const;
        size_t getCacheSizeBytes() const;
        std::vector<std::filesystem::path> getLoadedDirectories() const;
        bool isDirectoryLoaded(const std::string &subdirectory) const;
        size_t cleanupUnusedBuffers();
    };
} // namespace soundcoe