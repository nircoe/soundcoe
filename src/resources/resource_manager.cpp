#include <soundcoe/resources/resource_manager.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <soundcoe_config.hpp>
#include <algorithm>
#include <format>
#include <iterator>
#include <system_error>
#include <utility>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace internal
    {
        namespace
        {
            [[nodiscard]] std::unexpected<error> not_initialized(const std::string &method)
            {
                return std::unexpected(error_handler::make_error(error_code::not_initialized,
                    std::format("resource_manager::{}: resource_manager is not initialized", method)));
            }
        } // namespace

        resource_manager::resource_manager() : m_audio_context(), m_audio_root_directory(), m_source_pool(),
                                             m_free_source_indices(), m_buffer_cache(), m_loaded_directories() {}

        resource_manager::~resource_manager() { shutdown(); }

        std::expected<void, error> resource_manager::initialize(const std::string &audio_root_directory,
                                                                std::size_t max_sources,
                                                                std::size_t max_cache_size_mb)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_initialized)
            {
                logcoe::info("resource_manager::initialize: resource_manager is already initialized");
                return {};
            }

            if (audio_root_directory.empty())
                return std::unexpected(error_handler::make_error(error_code::invalid_argument,
                    "resource_manager::initialize: Audio root directory cannot be empty - "
                    "specify a valid directory path"));

            std::error_code ec;
            auto absolute_path = std::filesystem::absolute(audio_root_directory, ec);
            if (ec)
                return std::unexpected(error_handler::make_error(error_code::filesystem_error,
                    std::format("resource_manager::initialize: Failed to resolve \"{}\": {}", audio_root_directory,
                                ec.message())));

            if (auto r = m_audio_context.initialize(); !r)
                return r;

            m_audio_root_directory = absolute_path.lexically_normal();
            m_max_sources = max_sources;
            m_max_cache_size = max_cache_size_mb * 1024 * 1024;

            create_source_pool();

            m_initialized = true;
            logcoe::info("resource_manager::initialize: resource_manager initialized successfully");
            return {};
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

            if (auto r = m_audio_context.shutdown(); !r)
                logcoe::warning("resource_manager::shutdown: Failed to shutdown the audio_context: " +
                                r.error().message);

            m_current_cache_size = 0;
            m_initialized = false;
        }

        bool resource_manager::is_initialized() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_initialized;
        }

        std::expected<void, error> resource_manager::preload_directory(const std::string &subdirectory)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
                return not_initialized("preload_directory");

            if (subdirectory.empty())
                return std::unexpected(error_handler::make_error(error_code::invalid_argument,
                    "resource_manager::preload_directory: Cannot preload empty subdirectory - "
                    "this would load the entire audio root directory"));

            std::filesystem::path full_path = m_audio_root_directory / normalize_path(subdirectory);
            std::error_code ec;
            bool is_directory = std::filesystem::exists(full_path, ec) && std::filesystem::is_directory(full_path, ec);
            if (ec)
                return std::unexpected(error_handler::make_filesystem_error("resource_manager::preload_directory",
                                                                            subdirectory, ec));
            if (!is_directory)
                return std::unexpected(error_handler::make_error(error_code::directory_not_found,
                    "resource_manager::preload_directory: Not a directory: \"" + subdirectory + "\""));

            if (is_directory_loaded_impl(subdirectory))
            {
                logcoe::warning("resource_manager::preload_directory: Directory is already loaded: \"" + subdirectory + "\"");
                return {};
            }

            std::vector<std::filesystem::path> audio_files;
            if (scan_directory_for_files(subdirectory, audio_files))
            {
                // One bad file doesn't fail the whole scene
                for (const auto &file : audio_files)
                    static_cast<void>(preload_file_impl(file));

                m_loaded_directories.push_back(subdirectory);
                return {};
            }

            logcoe::warning("resource_manager::preload_directory: No audio files found in directory: " + subdirectory);
            return {};
        }

        std::expected<void, error> resource_manager::unload_directory(const std::string &subdirectory)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
                return not_initialized("unload_directory");

            if (subdirectory.empty())
            {
                logcoe::warning("resource_manager::unload_directory: Subdirectory path cannot be empty - specify a valid directory path");
                return {};
            }

            if (!is_directory_loaded_impl(subdirectory))
            {
                logcoe::warning("resource_manager::unload_directory: Directory is not loaded: \"" + subdirectory + "\"");
                return {};
            }

            std::vector<std::filesystem::path> audio_files;
            if (scan_directory_for_files(subdirectory, audio_files))
                for (const auto &file : audio_files)
                    unload_file_impl(file);
            else
                logcoe::warning("resource_manager::unload_directory: No audio files found in directory: " + subdirectory);

            m_loaded_directories.erase(std::remove(m_loaded_directories.begin(), m_loaded_directories.end(), subdirectory),
                                    m_loaded_directories.end());
            return {};
        }

        std::expected<std::reference_wrapper<sound_source>, error> resource_manager::acquire_source(
            std::size_t &pool_index, sound_priority priority)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
                return not_initialized("acquire_source");

            size_t index;
            if (m_free_source_indices.empty())
            {
                if (!find_source_to_replace(priority, index))
                    return std::unexpected(error_handler::make_error(error_code::resource_exhausted,
                        "resource_manager::acquire_source: Could not find a Source to replace"));
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
                return std::unexpected(error_handler::make_error(error_code::invalid_state,
                    "resource_manager::acquire_source: Source in the pool is null"));

            pool_index = index;
            return std::ref(*(entry.m_source));
        }

        std::expected<std::reference_wrapper<sound_buffer>, error> resource_manager::get_buffer(
            const std::string &filename)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
                return not_initialized("get_buffer");

            if (filename.empty())
                return std::unexpected(error_handler::make_error(error_code::invalid_argument,
                    "resource_manager::get_buffer: Filename cannot be empty - specify a valid audio file path"));

            std::filesystem::path found_path = find_file_in_loaded_directories(filename);
            if (found_path.empty())
                return std::unexpected(error_handler::make_error(error_code::file_not_found,
                    "resource_manager::get_buffer: No such file in the loaded directories: " + filename));

            std::string cache_key = found_path.lexically_normal().string();
            if (m_buffer_cache.find(cache_key) == m_buffer_cache.end())
            {
                if (auto r = preload_file_impl(found_path); !r)
                    return std::unexpected(r.error());
            }

            auto it = m_buffer_cache.find(cache_key);
            if (it == m_buffer_cache.end())
                return std::unexpected(error_handler::make_error(error_code::resource_exhausted,
                    "resource_manager::get_buffer: Buffer was evicted right after loading: " + filename));

            auto &entry = it->second;
            ++entry.m_reference_count;
            entry.m_last_accessed = std::chrono::steady_clock::now();
            return std::ref(*(entry.m_buffer));
        }

        void resource_manager::release_source(std::reference_wrapper<sound_source> source)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_initialized)
            {
                logcoe::error("resource_manager::release_source: resource_manager is not initialized");
                return;
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

                return;
            }

            logcoe::warning("resource_manager::release_source: This sound_source is not acquired");
        }

        void resource_manager::release_buffer(std::reference_wrapper<sound_buffer> buffer)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            release_buffer_impl(buffer.get().get_filename());
        }

        void resource_manager::release_buffer(const std::string &filename)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            release_buffer_impl(filename);
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
            std::error_code ec;
            std::filesystem::recursive_directory_iterator it(directory_full_path, ec);
            const std::filesystem::recursive_directory_iterator end;
            for (; !ec && it != end; it.increment(ec))
            {
                bool is_regular = it->is_regular_file(ec);
                if (ec)
                {
                    logcoe::warning("resource_manager::scan_directory_for_files: std::filesystem error for file: " +
                                    it->path().string() + ": " + ec.message());
                    continue;
                }

                if (is_regular)
                {
                    files.push_back(it->path());
                    found_file = true;
                }
            }
            if (ec)
                logcoe::warning("resource_manager::scan_directory_for_files: Failed to scan directory: " +
                                ec.message());

            std::string find_text = found_file ? "We found files" : "We didn't find files";
            logcoe::info("resource_manager::scan_directory_for_files: Finished, " + find_text + " in Directory: " + directory_full_path.string());
            return found_file;
        }

        std::expected<void, error> resource_manager::preload_file_impl(const std::filesystem::path &file_path)
        {
            if (!file_path.is_absolute())
                return std::unexpected(error_handler::make_error(error_code::invalid_argument,
                    "resource_manager::preload_file_impl: File path is not absolute: " + file_path.string()));

            std::error_code ec;
            bool is_file = std::filesystem::exists(file_path, ec) && std::filesystem::is_regular_file(file_path, ec);
            if (ec)
                return std::unexpected(error_handler::make_filesystem_error("resource_manager::preload_file_impl",
                                                                            file_path.string(), ec));
            if (!is_file)
                return std::unexpected(error_handler::make_error(error_code::file_not_found,
                    "resource_manager::preload_file_impl: Not a File: \"" + file_path.string() + "\""));

            std::string cache_key = file_path.string();
            if (m_buffer_cache.find(cache_key) != m_buffer_cache.end())
                return {};

            buffer_cache_entry entry;
            entry.m_buffer = std::make_unique<sound_buffer>();
            if (auto r = entry.m_buffer->load_from_file(cache_key); !r)
                return r;

            auto buffer_size = static_cast<std::size_t>(entry.m_buffer->get_size());
            if (buffer_size > m_max_cache_size)
                return std::unexpected(error_handler::make_error(error_code::resource_exhausted,
                    std::format("resource_manager::preload_file_impl: \"{}\" is {} bytes, larger than the cache "
                                "limit of {} bytes", cache_key, buffer_size, m_max_cache_size)));

            entry.m_reference_count = 0;
            entry.m_last_accessed = std::chrono::steady_clock::now();
            m_current_cache_size += entry.m_buffer->get_size();
            m_buffer_cache[cache_key] = std::move(entry);

            if (m_current_cache_size > m_max_cache_size)
                free_buffers();

            logcoe::info("resource_manager::preload_file_impl: preload_file Successfully: \"" + cache_key + "\"");
            return {};
        }

        void resource_manager::unload_file_impl(const std::filesystem::path &file_path)
        {
            std::error_code ec;
            bool is_file = std::filesystem::exists(file_path, ec) && std::filesystem::is_regular_file(file_path, ec);
            if (ec)
            {
                logcoe::warning("resource_manager::unload_file_impl: Failed to check \"" + file_path.string() +
                                "\": " + ec.message());
                return;
            }
            if (!is_file)
            {
                logcoe::warning("resource_manager::unload_file_impl: Not a File: \"" + file_path.string() + "\"");
                return;
            }

            std::string cache_key = file_path.string();
            if (m_buffer_cache.find(cache_key) == m_buffer_cache.end())
            {
                logcoe::warning("resource_manager::unload_file_impl: File is not loaded: \"" + cache_key + "\"");
                return;
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

        void resource_manager::release_buffer_impl(const std::string &filename)
        {
            if (!m_initialized)
            {
                logcoe::error("resource_manager::release_buffer_impl: resource_manager is not initialized");
                return;
            }

            std::filesystem::path found_path = find_file_in_loaded_directories(filename);
            if(found_path.empty())
            {
                logcoe::warning("resource_manager::release_buffer_impl: No such file found for release: " + filename);
                return;
            }

            std::string cache_key = found_path.lexically_normal().string();
            if (m_buffer_cache.find(cache_key) == m_buffer_cache.end())
            {
                logcoe::warning("resource_manager::release_buffer_impl: Buffer is not loaded in cache: " + cache_key);
                return;
            }

            auto &entry = m_buffer_cache[cache_key];
            if (entry.m_reference_count == 0)
            {
                logcoe::warning("resource_manager::release_buffer_impl: Not a single Source is using this Buffer at the moment");
                return;
            }

            --entry.m_reference_count;
        }

        std::filesystem::path resource_manager::find_file_in_loaded_directories(const std::string &filename) const
        {
            auto loaded_dirs = m_loaded_directories;
            std::error_code ec;
            for(const auto &dir : loaded_dirs)
            {
                std::filesystem::path candidate_path = (m_audio_root_directory / dir / filename).lexically_normal();
                bool is_file = std::filesystem::exists(candidate_path, ec) &&
                               std::filesystem::is_regular_file(candidate_path, ec);
                if (ec)
                {
                    logcoe::warning("resource_manager::find_file_in_loaded_directories: Failed to check \"" +
                                    candidate_path.string() + "\": " + ec.message());
                    continue;
                }

                if (is_file)
                    return candidate_path;
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
