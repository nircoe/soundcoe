#pragma once

#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error.hpp>
#include <soundcoe/core/types.hpp>
#include <soundcoe/resources/sound_buffer.hpp>
#include <soundcoe/resources/sound_source.hpp>
#include <AL/al.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <deque>
#include <expected>
#include <filesystem>
#include <optional>
#include <functional>
#include <chrono>
#include <cstddef>

namespace soundcoe
{
    namespace internal
    {
        struct source_allocation
        {
            std::unique_ptr<sound_source> m_source;
            sound_priority m_priority;
            std::chrono::steady_clock::time_point m_allocated_time;
            bool m_active;
        };

        struct buffer_cache_entry
        {
            std::unique_ptr<sound_buffer> m_buffer;
            size_t m_reference_count;
            std::chrono::steady_clock::time_point m_last_accessed;
        };

        class resource_manager
        {
            audio_context m_audio_context;
            bool m_initialized = false;
            std::filesystem::path m_audio_root_directory;
            size_t m_max_sources = 64;
            mutable std::mutex m_mutex;

            std::vector<source_allocation> m_source_pool;
            std::deque<size_t> m_free_source_indices;

            std::unordered_map<std::string, buffer_cache_entry> m_buffer_cache;
            size_t m_max_cache_size = 64 * 1024 * 1024; // 64MB
            size_t m_current_cache_size = 0;

            std::vector<std::filesystem::path> m_loaded_directories;

            void create_source_pool();
            bool find_source_to_replace(sound_priority new_priority, size_t &replace_index);
            void free_buffers();
            std::filesystem::path normalize_path(const std::string &path) const;
            bool scan_directory_for_files(const std::filesystem::path &subdirectory, std::vector<std::filesystem::path> &files);
            [[nodiscard]] std::expected<void, error> preload_file_impl(const std::filesystem::path &file_path);
            [[nodiscard]] std::expected<void, error> unload_file_impl(const std::filesystem::path &file_path);
            bool is_directory_loaded_impl(const std::string &subdirectory) const;
            sound_priority get_highest_priority_for_buffer(ALuint buffer_id) const;
            void release_buffer_impl(const std::string &filename);
            std::filesystem::path find_file_in_loaded_directories(const std::string &filename) const;

        public:
            resource_manager();
            ~resource_manager();

            [[nodiscard]] std::expected<void, error> initialize(const std::string &audio_root_directory,
                                                                std::size_t max_sources = 64,
                                                                std::size_t max_cache_size_mb = UNLIMITED_CACHE);
            void shutdown();
            bool is_initialized() const;

            [[nodiscard]] std::expected<void, error> preload_directory(const std::string &subdirectory);
            [[nodiscard]] std::expected<void, error> unload_directory(const std::string &subdirectory);

            [[nodiscard]] std::expected<std::reference_wrapper<sound_source>, error> acquire_source(
                std::size_t &pool_index, sound_priority priority = sound_priority::medium);
            [[nodiscard]] std::expected<std::reference_wrapper<sound_buffer>, error> get_buffer(
                const std::string &filename);
            void release_source(std::reference_wrapper<sound_source> source);
            void release_buffer(std::reference_wrapper<sound_buffer> buffer);
            void release_buffer(const std::string &filename);

            size_t get_active_source_count() const;
            size_t get_total_source_count() const;
            size_t get_cached_buffer_count() const;
            size_t get_cache_size_bytes() const;
            std::vector<std::filesystem::path> get_loaded_directories() const;
            bool is_directory_loaded(const std::string &subdirectory) const;
            size_t cleanup_unused_buffers();

            std::optional<std::reference_wrapper<source_allocation>> get_source_allocation(size_t index);
        };
    } // namespace internal
} // namespace soundcoe
