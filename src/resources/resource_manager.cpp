#include <soundcoe/resources/resource_manager.hpp>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <iterator>
#include <utility>
#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace internal
    {
        resource_manager::resource_manager() : m_audio_context(), m_audio_root_directory(), m_source_pool(),
                                             m_free_source_indices(), m_buffer_cache(), m_loaded_directories() {}

        resource_manager::~resource_manager() { shutdown(); }

        void resource_manager::initialize(const std::string &audio_root_directory, size_t max_sources,
                                        size_t max_cache_size_mb)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_initialized)
            {
                logcoe::info("resource_manager::initialize: resource_manager is already initialized");
                return;
            }

            if (audio_root_directory.empty())
            {
                logcoe::warning("resource_manager::initialize: Audio root directory cannot be empty - specify a valid directory path");
                return;
            }

            m_audio_context.initialize();

            m_audio_root_directory = std::filesystem::absolute(audio_root_directory).lexically_normal();
            m_max_sources = max_sources;
            m_max_cache_size = max_cache_size_mb * 1024 * 1024;

            try
            {
                create_source_pool();
            }
            catch (const std::exception &e)
            {
                logcoe::error("resource_manager::initialize: Failed to create Source Pool: " + std::string(e.what()));
                m_source_pool.clear();
                m_free_source_indices.clear();
                throw;
            }

            m_initialized = true;
            logcoe::info("resource_manager::initialize: resource_manager initialized successfully");
        }

        void resource_manager::shutdown()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (!m_initialized)
                return;

            m_source_pool.clear();
            m_buffer_cache.clear();
            m_loaded_directories.clear();
            m_free_source_indices.clear();

            try { m_audio_context.shutdown(); }
            catch(const std::runtime_error &) { logcoe::warning("resource_manager::shutdown: Failed to shutdown the audio_context"); }

            m_current_cache_size = 0;
            m_initialized = false;
        }

        bool resource_manager::is_initialized() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_initialized;
        }

        bool resource_manager::preload_directory(const std::string &subdirectory)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::preload_directory: resource_manager is not initialized");
                return false;
            }

            if (subdirectory.empty())
            {
                logcoe::warning("resource_manager::preload_directory: Cannot preload empty subdirectory - this would load the entire audio root directory");
                return false;
            }

            std::filesystem::path full_path = m_audio_root_directory / normalize_path(subdirectory);
            if (!std::filesystem::exists(full_path) || !std::filesystem::is_directory(full_path))
            {
                logcoe::warning("resource_manager::preload_directory: Not a directory: \"" + subdirectory + "\"");
                return false;
            }

            if (is_directory_loaded_impl(subdirectory))
            {
                logcoe::warning("resource_manager::preload_directory: Directory is already loaded: \"" + subdirectory + "\"");
                return true;
            }

            std::vector<std::filesystem::path> audio_files;
            if (scan_directory_for_files(subdirectory, audio_files))
            {
                for (const auto &file : audio_files)
                    preload_file_impl(file);

                m_loaded_directories.push_back(subdirectory);
                return true;
            }

            logcoe::warning("resource_manager::preload_directory: No audio files found in directory: " + subdirectory);
            return true;
        }

        bool resource_manager::unload_directory(const std::string &subdirectory)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::unload_directory: resource_manager is not initialized");
                return false;
            }

            if (subdirectory.empty())
            {
                logcoe::warning("resource_manager::unload_directory: Subdirectory path cannot be empty - specify a valid directory path");
                return true;
            }

            if (!is_directory_loaded_impl(subdirectory))
            {
                logcoe::warning("resource_manager::unload_directory: Directory is not loaded: \"" + subdirectory + "\"");
                return true;
            }

            std::vector<std::filesystem::path> audio_files;
            if (scan_directory_for_files(subdirectory, audio_files))
                for (const auto &file : audio_files)
                    unload_file_impl(file);
            else
                logcoe::warning("resource_manager::unload_directory: No audio files found in directory: " + subdirectory);

            m_loaded_directories.erase(std::remove(m_loaded_directories.begin(), m_loaded_directories.end(), subdirectory),
                                    m_loaded_directories.end());
            return true;
        }

        std::optional<std::reference_wrapper<sound_source>> resource_manager::acquire_source(size_t &pool_index, sound_priority priority)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::acquire_source: resource_manager is not initialized");
                return std::nullopt;
            }

            size_t index;
            if (m_free_source_indices.empty())
            {
                if (!find_source_to_replace(priority, index))
                {
                    logcoe::error("resource_manager::acquire_source: Could not find a Source to replace");
                    return std::nullopt;
                }
            }
            else
            {
                index = m_free_source_indices.front();
                m_free_source_indices.pop_front();
            }

            auto &entry = m_source_pool[index];
            entry.m_priority = priority;
            entry.m_allocated_time = std::chrono::steady_clock::now();
            entry.m_active = true;

            if (entry.m_source.get() == nullptr)
                return std::nullopt;

            pool_index = index;
            return std::ref(*(entry.m_source));
        }

        std::optional<std::reference_wrapper<sound_buffer>> resource_manager::get_buffer(const std::string &filename)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::get_buffer: resource_manager is not initialized");
                return std::nullopt;
            }

            if (filename.empty())
            {
                logcoe::error("resource_manager::get_buffer: Filename cannot be empty - specify a valid audio file path");
                return std::nullopt;
            }
            std::filesystem::path found_path = find_file_in_loaded_directories(filename);
            if(found_path.empty())
            {
                logcoe::error("resource_manager::get_buffer: No such file in the loaded directories: " + filename);
                return std::nullopt;
            }

            std::string cache_key = found_path.lexically_normal().string();
            if ((m_buffer_cache.find(cache_key) == m_buffer_cache.end()) && !preload_file_impl(found_path))
                return std::nullopt;

            auto &entry = m_buffer_cache[cache_key];
            ++entry.m_reference_count;
            entry.m_last_accessed = std::chrono::steady_clock::now();
            return std::ref(*(entry.m_buffer));
        }

        bool resource_manager::release_source(std::reference_wrapper<sound_source> source)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::release_source: resource_manager is not initialized");
                return false;
            }

            for (size_t i = 0; i < m_source_pool.size(); ++i)
            {
                auto &allocation = m_source_pool[i];
                if (!allocation.m_active || allocation.m_source->get_source_id() != source.get().get_source_id())
                    continue;

                if (auto r = allocation.m_source->detach_buffer(); !r)
                    logcoe::warning("resource_manager::release_source: Failed to detach Buffer: " + r.error().message);

                allocation.m_active = false;
                m_free_source_indices.push_back(i);

                return true;
            }

            logcoe::warning("resource_manager::release_source: This sound_source is not acquired");
            return true;
        }

        bool resource_manager::release_buffer(std::reference_wrapper<sound_buffer> buffer)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return release_buffer_impl(buffer.get().get_filename());
        }

        bool resource_manager::release_buffer(const std::string &filename)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return release_buffer_impl(filename);
        }

        size_t resource_manager::get_active_source_count() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::get_active_source_count: resource_manager is not initialized");
                return 0;
            }

            return m_source_pool.size() - m_free_source_indices.size();
        }

        size_t resource_manager::get_total_source_count() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::get_total_source_count: resource_manager is not initialized");
                return 0;
            }

            return m_source_pool.size();
        }

        size_t resource_manager::get_cached_buffer_count() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::get_cached_buffer_count: resource_manager is not initialized");
                return 0;
            }

            return m_buffer_cache.size();
        }

        size_t resource_manager::get_cache_size_bytes() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::get_cache_size_bytes: resource_manager is not initialized");
                return 0;
            }

            return m_current_cache_size;
        }

        std::vector<std::filesystem::path> resource_manager::get_loaded_directories() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::get_loaded_directories: resource_manager is not initialized");
                return {};
            }

            return m_loaded_directories;
        }

        bool resource_manager::is_directory_loaded(const std::string &subdirectory) const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::is_directory_loaded: resource_manager is not initialized");
                return false;
            }

            if (subdirectory.empty())
            {
                logcoe::warning("resource_manager::is_directory_loaded: Subdirectory cannot be empty - specify a valid directory path");
                return false;
            }

            return is_directory_loaded_impl(subdirectory);
        }

        size_t resource_manager::cleanup_unused_buffers()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::cleanup_unused_buffers: resource_manager is not initialized");
                return 0;
            }

            size_t removed = 0;
            for (auto it = m_buffer_cache.begin(); it != m_buffer_cache.end();)
            {
                if (it->second.m_reference_count == 0)
                {
                    m_current_cache_size -= it->second.m_buffer->get_size();
                    it = m_buffer_cache.erase(it);
                    ++removed;
                }
                else
                    ++it;
            }

            return removed;
        }

        void resource_manager::create_source_pool()
        {
            m_source_pool.resize(m_max_sources);
            m_free_source_indices.clear();
            auto time = std::chrono::steady_clock::now();

            for (size_t index = 0; index < m_max_sources; ++index)
            {
                m_source_pool[index].m_source = std::make_unique<sound_source>();
                m_source_pool[index].m_priority = sound_priority::medium;
                m_source_pool[index].m_allocated_time = time;
                m_source_pool[index].m_active = false;
                m_free_source_indices.push_back(index);
            }
        }

        bool resource_manager::find_source_to_replace(sound_priority new_priority, size_t &replace_index)
        {
            if (m_source_pool.empty())
            {
                logcoe::warning("resource_manager::find_source_to_replace: Source Pool is empty");
                return false;
            }

            if (!m_free_source_indices.empty())
            {
                logcoe::warning("resource_manager::find_source_to_replace: There are free sources available");
                return false;
            }

            auto source_to_replace = std::find_if(m_source_pool.begin(), m_source_pool.end(),
                                                [](const auto &it)
                                                {
                                                    return it.m_active && it.m_source->is_stopped();
                                                });
            if (source_to_replace == m_source_pool.end())
            {
                source_to_replace = std::min_element(m_source_pool.begin(), m_source_pool.end(),
                                                [](const auto &a, const auto &b)
                                                {
                                                    if (a.m_priority != b.m_priority)
                                                        return a.m_priority < b.m_priority;
                                                    return a.m_allocated_time < b.m_allocated_time;
                                                });

                if (source_to_replace->m_priority > new_priority)
                {
                    logcoe::debug("resource_manager::find_source_to_replace: There is no lower priority source to replace with");
                    return false;
                }

                if (source_to_replace->m_active)
                {
                    static_cast<void>(source_to_replace->m_source->stop());
                    source_to_replace->m_active = false;
                }
            }

            replace_index = std::distance(m_source_pool.begin(), source_to_replace);
            logcoe::info("resource_manager::find_source_to_replace: Finished successfully");
            return true;
        }

        void resource_manager::free_buffers()
        {
            while (m_current_cache_size > m_max_cache_size && !m_buffer_cache.empty())
            {
                auto to_free = std::min_element(m_buffer_cache.begin(), m_buffer_cache.end(),
                                            [this](const auto &a, const auto &b)
                                            {
                                                if (a.second.m_reference_count == 0 && b.second.m_reference_count > 0)
                                                    return true;
                                                if (a.second.m_reference_count > 0 && b.second.m_reference_count == 0)
                                                    return false;

                                                auto a_priority = get_highest_priority_for_buffer(a.second.m_buffer->get_buffer_id());
                                                auto b_priority = get_highest_priority_for_buffer(b.second.m_buffer->get_buffer_id());
                                                if (a_priority != b_priority)
                                                    return a_priority < b_priority;

                                                return a.second.m_last_accessed < b.second.m_last_accessed;
                                            });

                auto oldest_buffer_id = to_free->second.m_buffer->get_buffer_id();
                for (size_t i = 0; i < m_source_pool.size(); ++i)
                {
                    auto &allocation = m_source_pool[i];
                    if (!allocation.m_active || allocation.m_source->get_buffer_id() != oldest_buffer_id)
                        continue;

                    if (auto r = allocation.m_source->detach_buffer(); !r)
                        logcoe::warning("resource_manager::free_buffers: Failed to detach Buffer: " +
                                        r.error().message);

                    allocation.m_active = false;
                    m_free_source_indices.push_back(i);
                }

                m_current_cache_size -= to_free->second.m_buffer->get_size();
                m_buffer_cache.erase(to_free);
            }
        }

        std::filesystem::path resource_manager::normalize_path(const std::string &path) const
        {
            return std::filesystem::path(path).lexically_normal();
        }

        bool resource_manager::scan_directory_for_files(const std::filesystem::path &subdirectory,
                                                    std::vector<std::filesystem::path> &files)
        {
            auto directory_full_path = m_audio_root_directory / subdirectory.lexically_normal();
            bool found_file = false;
            try
            {
                for (const auto &entry : std::filesystem::recursive_directory_iterator(directory_full_path))
                {
                    try
                    {
                        if (entry.is_regular_file())
                        {
                            files.push_back(entry.path());
                            found_file = true;
                        }
                    }
                    catch (const std::filesystem::filesystem_error &e)
                    {
                        logcoe::warning("resource_manager::scan_directory_for_files: std::filesystem exception for file: " + entry.path().string() + ": " + std::string(e.what()));
                    }
                }
            }
            catch (const std::exception &e)
            {
                logcoe::warning("resource_manager::scan_directory_for_files: Failed to scan directory: " + std::string(e.what()));
            }

            std::string find_text = found_file ? "We found files" : "We didn't find files";
            logcoe::info("resource_manager::scan_directory_for_files: Finished, " + find_text + " in Directory: " + directory_full_path.string());
            return found_file;
        }

        bool resource_manager::preload_file_impl(const std::filesystem::path &file_path)
        {
            if (!file_path.is_absolute())
            {
                logcoe::error("resource_manager::preload_file_impl: File path is not absolute: " + file_path.string());
                return false;
            }

            try
            {
                if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path))
                {
                    logcoe::error("resource_manager::preload_file_impl: Not a File: \"" + file_path.string() + "\"");
                    return false;
                }
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                logcoe::error("resource_manager::preload_file_impl: std::filesystem exception: " + std::string(e.what()));
                return false;
            }

            std::string cache_key = file_path.string();
            if (m_buffer_cache.find(cache_key) != m_buffer_cache.end())
                return true;

            buffer_cache_entry entry;
            entry.m_buffer = std::make_unique<sound_buffer>();
            if (!entry.m_buffer->load_from_file(cache_key))
                return false;
            entry.m_reference_count = 0;
            entry.m_last_accessed = std::chrono::steady_clock::now();
            m_current_cache_size += entry.m_buffer->get_size();
            m_buffer_cache[cache_key] = std::move(entry);

            if (m_current_cache_size > m_max_cache_size)
                free_buffers();

            logcoe::info("resource_manager::preload_file_impl: preload_file Successfully: \"" + cache_key + "\"");
            return true;
        }

        bool resource_manager::unload_file_impl(const std::filesystem::path &file_path)
        {
            try
            {
                if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path))
                {
                    logcoe::warning("resource_manager::unload_file_impl: Not a File: \"" + file_path.string() + "\"");
                    return true;
                }
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                logcoe::error("resource_manager::unload_file_impl: std::filesystem exception: " + std::string(e.what()));
                return false;
            }

            std::string cache_key = file_path.string();
            if (m_buffer_cache.find(cache_key) == m_buffer_cache.end())
            {
                logcoe::warning("resource_manager::unload_file_impl: File is not loaded: \"" + cache_key + "\"");
                return true;
            }

            auto &entry = m_buffer_cache[cache_key];
            if (entry.m_reference_count > 0)
            {
                auto buffer_id = entry.m_buffer->get_buffer_id();
                for (size_t i = 0; i < m_source_pool.size(); ++i)
                {
                    auto &allocation = m_source_pool[i];
                    if (!allocation.m_active || allocation.m_source->get_buffer_id() != buffer_id)
                        continue;

                    if (auto r = allocation.m_source->detach_buffer(); !r)
                        logcoe::warning("resource_manager::unload_file_impl: Failed to detach Buffer: " +
                                        r.error().message);

                    allocation.m_active = false;
                    m_free_source_indices.push_back(i);
                }
            }

            m_current_cache_size -= entry.m_buffer->get_size();
            m_buffer_cache.erase(cache_key);
            return true;
        }

        bool resource_manager::is_directory_loaded_impl(const std::string &subdirectory) const
        {
            auto it = std::find(m_loaded_directories.begin(), m_loaded_directories.end(), subdirectory);

            return it != m_loaded_directories.end();
        }

        sound_priority resource_manager::get_highest_priority_for_buffer(ALuint buffer_id) const
        {
            sound_priority highest = sound_priority::low;
            for (const auto &allocation : m_source_pool)
            {
                if (allocation.m_active && allocation.m_source->get_buffer_id() == buffer_id &&
                    allocation.m_priority > highest)
                {
                    highest = allocation.m_priority;
                }
            }

            return highest;
        }

        bool resource_manager::release_buffer_impl(const std::string &filename)
        {
            if (!m_initialized)
            {
                logcoe::error("resource_manager::release_buffer_impl: resource_manager is not initialized");
                return false;
            }

            std::filesystem::path found_path = find_file_in_loaded_directories(filename);
            if(found_path.empty())
            {
                logcoe::warning("resource_manager::release_buffer_impl: No such file found for release: " + filename);
                return true;
            }

            std::string cache_key = found_path.lexically_normal().string();
            if (m_buffer_cache.find(cache_key) == m_buffer_cache.end())
            {
                logcoe::warning("resource_manager::release_buffer_impl: Buffer is not loaded in cache: " + cache_key);
                return true;
            }

            auto &entry = m_buffer_cache[cache_key];
            if (entry.m_reference_count == 0)
            {
                logcoe::warning("resource_manager::release_buffer_impl: Not a single Source is using this Buffer at the moment");
                return true;
            }

            --entry.m_reference_count;
            return true;
        }

        std::filesystem::path resource_manager::find_file_in_loaded_directories(const std::string &filename) const
        {
            auto loaded_dirs = m_loaded_directories;
            for(const auto &dir : loaded_dirs)
            {
                std::filesystem::path candidate_path = (m_audio_root_directory / dir / filename).lexically_normal();
                if (std::filesystem::exists(candidate_path) && std::filesystem::is_regular_file(candidate_path))
                {
                    return candidate_path;
                }
            }
            return std::filesystem::path();
        }

        std::optional<std::reference_wrapper<source_allocation>> resource_manager::get_source_allocation(size_t index)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (index < m_source_pool.size())
                return std::ref(m_source_pool[index]);
            return std::nullopt;
        }
    } // namespace internal
} // namespace soundcoe
