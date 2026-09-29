#include <soundcoe/playback/sound_manager.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <AL/al.h>
#include <functional>
#include <filesystem>
#include <algorithm>
#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace detail
    {
        void sound_manager::update_all_sounds_volume()
        {
            update_all_audio_property(m_active_sounds, [](std::unique_ptr<sound_source> &audio, float value)
                                   { audio->set_volume(value); }, [&](const active_audio &audio)
                                   { return (m_sounds_mute || m_mute) ? 0.0f : audio.m_base_volume; }, m_master_volume, m_master_sounds_volume);
        }

        void sound_manager::update_all_music_volume()
        {
            update_all_audio_property(m_active_music, [](std::unique_ptr<sound_source> &audio, float value)
                                   { audio->set_volume(value); }, [&](const active_audio &audio)
                                   { return (m_music_mute || m_mute) ? 0.0f : audio.m_base_volume; }, m_master_volume, m_master_music_volume);
        }

        void sound_manager::update_all_volume()
        {
            update_all_sounds_volume();
            update_all_music_volume();
        }

        void sound_manager::update_all_sounds_pitch()
        {
            update_all_audio_property(m_active_sounds, [](std::unique_ptr<sound_source> &audio, float value)
                                   { audio->set_pitch(value); }, [](const active_audio &audio)
                                   { return audio.m_base_pitch; }, m_master_pitch, m_master_sounds_pitch);
        }

        void sound_manager::update_all_music_pitch()
        {
            update_all_audio_property(m_active_music, [](std::unique_ptr<sound_source> &audio, float value)
                                   { audio->set_pitch(value); }, [](const active_audio &audio)
                                   { return audio.m_base_pitch; }, m_master_pitch, m_master_music_pitch);
        }

        void sound_manager::update_all_pitch()
        {
            update_all_sounds_pitch();
            update_all_music_pitch();
        }

        bool sound_manager::set_listener_position_impl(const vec3 &position)
        {
            ALfloat pos[3] = {position.x, position.y, position.z};
            alListenerfv(AL_POSITION, pos);
            if (error_handler::check_openal_error("Set Listener Position"))
                return false;
            m_listener_position = position;
            return true;
        }

        bool sound_manager::set_listener_velocity_impl(const vec3 &velocity)
        {
            ALfloat vel[3] = {velocity.x, velocity.y, velocity.z};
            alListenerfv(AL_VELOCITY, vel);
            if (error_handler::check_openal_error("Set Listener Velocity"))
                return false;
            m_listener_velocity = velocity;
            return true;
        }

        bool sound_manager::set_listener_orientation_impl(const vec3 &forward, const vec3 &up)
        {
            ALfloat orientation[6] = {forward.x, forward.y, forward.z,
                                      up.x, up.y, up.z};
            alListenerfv(AL_ORIENTATION, orientation);
            if (error_handler::check_openal_error("Set Listener Forward and Up Vectors"))
                return false;
            m_listener_forward = forward;
            m_listener_up = up;
            return true;
        }

        bool sound_manager::set_error(const std::string &error)
        {
            m_has_error = true;
            m_last_error = error;
            logcoe::error(error);
            return false;
        }

        bool sound_manager::fade_to_volume(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                                        float target_volume, float duration, const std::string &method)
        {
            auto it = active_audio_.find(handle);
            if (it == active_audio_.end())
                return set_error("sound_manager::" + method + ": Invalid handle");

            if (duration <= 0.0f)
                return set_error("sound_manager::" + method + ": Fade duration must be positive");

            if (target_volume < 0.0f)
                return set_error("sound_manager::" + method + ": Fade target volume must be non-negative");

            active_audio &audio = it->second;

            auto source_allocation = m_resource_manager.get_source_allocation(audio.m_source_index);
            if (!(source_allocation.has_value()) || !(source_allocation.value().get().m_active))
            {
                active_audio_.erase(it);
                return set_error("sound_manager::" + method + ": Audio source is no longer active");
            }

            auto &source = source_allocation.value().get().m_source;
            if (!(source->is_playing()))
                return set_error("sound_manager::" + method + ": Cannot fade_to_volume audio that is not playing.");

            audio.m_is_fading = true;
            audio.m_fade_start_volume = audio.m_base_volume;
            audio.m_fade_target_volume = target_volume;
            audio.m_fade_duration = duration;
            audio.m_fade_elapsed = 0.0f;
            return true;
        }

        bool sound_manager::fade(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                                bool fade_in, float duration, const std::string &method)
        {
            auto it = active_audio_.find(handle);
            if (it == active_audio_.end())
                return set_error("sound_manager::" + method + ": Invalid handle");

            if (duration <= 0.0f)
                return set_error("sound_manager::" + method + ": Fade duration must be positive");

            active_audio &audio = it->second;

            auto source_allocation = m_resource_manager.get_source_allocation(audio.m_source_index);
            if (!(source_allocation.has_value()) || !(source_allocation.value().get().m_active))
            {
                active_audio_.erase(it);
                return set_error("sound_manager::" + method + ": Audio source is no longer active");
            }

            auto &source = source_allocation.value().get().m_source;
            if (!fade_in && !(source->is_playing()))
                return set_error("sound_manager::" + method + ": Cannot fade out audio that is not playing.");

            audio.m_is_fading = true;
            audio.m_fade_start_volume = fade_in ? 0.0f : audio.m_base_volume;
            audio.m_fade_target_volume = fade_in ? audio.m_base_volume : 0.0f;
            audio.m_fade_duration = duration;
            audio.m_fade_elapsed = 0.0f;
            return true;
        }

        bool sound_manager::check_audio_state(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                                           sound_state state, const std::string &method)
        {
            m_last_error = "";
            m_has_error = false;

            auto it = active_audio_.find(handle);
            if (it == active_audio_.end())
                return set_error("sound_manager::" + method + ": Invalid handle");

            active_audio &audio = it->second;
            auto source_allocation = m_resource_manager.get_source_allocation(audio.m_source_index);
            if (!(source_allocation.has_value()) || !(source_allocation.value().get().m_active))
            {
                active_audio_.erase(it);
                return set_error("sound_manager::" + method + ": Audio source is no longer active");
            }

            auto &source = source_allocation.value().get().m_source;
            if (state == sound_state::playing)
                return source->is_playing();
            if (state == sound_state::paused)
                return source->is_paused();
            if (state == sound_state::stopped)
                return source->is_stopped();

            return set_error("sound_manager::" + method + ": Internal error - Invalid operation type");
        }

        bool sound_manager::set_audio_property(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                                            property_type type, const std::string &method,
                                            float value, float y, float z)
        {
            auto it = active_audio_.find(handle);
            if (it == active_audio_.end())
                return set_error("sound_manager::" + method + ": Invalid handle");

            active_audio &audio = it->second;
            auto source_allocation = m_resource_manager.get_source_allocation(audio.m_source_index);
            if (!(source_allocation.has_value()) || !(source_allocation.value().get().m_active))
            {
                active_audio_.erase(it);
                return set_error("sound_manager::" + method + ": Audio source is no longer active");
            }

            auto &source = source_allocation.value().get().m_source;
            vec3 vec;
            if (type == property_type::position || type == property_type::velocity)
                vec = {value, y, z};

            if (type == property_type::volume)
                return source->set_volume(value);
            if (type == property_type::pitch)
                return source->set_pitch(value);
            if (type == property_type::position)
                return source->set_position(vec);
            if (type == property_type::velocity)
                return source->set_velocity(vec);

            return set_error("sound_manager::" + method + ": Internal error - Invalid property_type");
        }

        bool sound_manager::audio_operation(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                                          sound_state operation, const std::string &method)
        {
            auto it = active_audio_.find(handle);
            if (it == active_audio_.end())
                return set_error("sound_manager::" + method + ": Invalid handle");

            active_audio &audio = it->second;
            auto source_allocation = m_resource_manager.get_source_allocation(audio.m_source_index);
            if (!(source_allocation.has_value()) || !(source_allocation.value().get().m_active))
            {
                active_audio_.erase(it);
                return set_error("sound_manager::" + method + ": Audio source is no longer active");
            }

            auto &source = source_allocation.value().get().m_source;
            if (operation == sound_state::playing)
                return source->play();
            if (operation == sound_state::paused)
                return source->pause();
            if (operation == sound_state::stopped)
            {
                bool succeed = source->stop();
                if (succeed)
                {
                    m_resource_manager.release_source(*source);
                    m_resource_manager.release_buffer(audio.m_filename);
                    active_audio_.erase(it);
                }
                return succeed;
            }

            return set_error("sound_manager::" + method + ": Internal error - Invalid operation type");
        }

        bool sound_manager::audio_operation_all(std::unordered_map<size_t, active_audio> &active_audio_, sound_state operation,
                                             const std::string &method)
        {
            for (auto it = active_audio_.begin(); it != active_audio_.end();)
            {
                active_audio &audio = it->second;
                auto source_allocation = m_resource_manager.get_source_allocation(audio.m_source_index);
                if (!(source_allocation.has_value()) || !(source_allocation.value().get().m_active))
                {
                    logcoe::warning("sound_manager::" + method + ": handle " + std::to_string(it->first) + " is no longer active");
                    it = active_audio_.erase(it);
                    continue;
                }

                auto &source = source_allocation.value().get().m_source;
                bool success = false;
                if (operation == sound_state::playing)
                {
                    if (source->is_paused())
                        success = source->play();
                    else
                        logcoe::warning("sound_manager::" + method + ": handle " + std::to_string(it->first) + " is not paused");
                }
                else if (operation == sound_state::paused)
                    success = source->pause();
                else if (operation == sound_state::stopped)
                {
                    success = source->stop();
                    if (success)
                    {
                        m_resource_manager.release_source(*source);
                        m_resource_manager.release_buffer(audio.m_filename);
                        it = active_audio_.erase(it);
                        continue;
                    }
                }
                else
                    return set_error("sound_manager::" + method + ": Internal error - Invalid operation type");

                if (!success)
                    logcoe::warning("sound_manager::" + method + ": Failed to operate on handle - " + std::to_string(it->first));

                ++it;
            }

            return true;
        }

        size_t sound_manager::play(std::unordered_map<size_t, active_audio> &active_audio_, const std::string &filename,
                                  float volume, float pitch, bool loop, sound_priority priority,
                                  std::atomic<size_t> &next_handle, const std::string &method,
                                  float master_category_volume, float master_category_pitch,
                                  bool is_3d, const vec3 &position, const vec3 &velocity)
        {
            auto buffer = m_resource_manager.get_buffer(filename);
            if (!(buffer.has_value()))
            {
                logcoe::error("sound_manager::" + method + ": Failed to load the sound file");
                return INVALID_SOUND_HANDLE;
            }

            size_t pool_index;
            auto source = m_resource_manager.acquire_source(pool_index, priority);
            if (!(source.has_value()))
            {
                logcoe::error("sound_manager::" + method + ": Failed to acquire source");
                m_resource_manager.release_buffer(buffer.value());
                return INVALID_SOUND_HANDLE;
            }

            try
            {
                source->get().attach_buffer(buffer->get());
            }
            catch (const std::exception &e)
            {
                logcoe::error("sound_manager::" + method + ": Failed to attach buffer: " + std::string(e.what()));
                m_resource_manager.release_source(source.value());
                m_resource_manager.release_buffer(buffer.value());
                return INVALID_SOUND_HANDLE;
            }

            if (!(source->get().set_volume(volume * m_master_volume * master_category_volume)))
                logcoe::warning("sound_manager::" + method + ": Failed to set volume for " + filename);
            if (!(source->get().set_pitch(pitch * m_master_pitch * master_category_pitch)))
                logcoe::warning("sound_manager::" + method + ": Failed to set pitch for " + filename);
            if (!(source->get().set_looping(loop)))
                logcoe::warning("sound_manager::" + method + ": Failed to set looping for " + filename);
            if (is_3d)
            {
                if (!(source->get().set_position(position)))
                    logcoe::warning("sound_manager::" + method + ": Failed to set position for " + filename);
                if (!(source->get().set_velocity(velocity)))
                    logcoe::warning("sound_manager::" + method + ": Failed to set velocity for " + filename);
            }
            if (!(source->get().play()))
            {
                logcoe::error("sound_manager::" + method + ": Failed to play the sound " + filename);
                m_resource_manager.release_source(source.value());
                m_resource_manager.release_buffer(buffer.value());
                return INVALID_SOUND_HANDLE;
            }

            active_audio audio;
            audio.m_source_index = pool_index;
            audio.m_filename = filename;
            audio.m_base_volume = volume;
            audio.m_base_pitch = pitch;
            audio.m_loop = loop;
            audio.m_stream = buffer->get().is_streaming();

            active_audio_[next_handle] = std::move(audio);

            return next_handle++;
        }

        void sound_manager::handle_streaming_audio()
        {
            // In the roadmap, not yet implemented
        }

        void sound_manager::handle_fade_effects(std::unordered_map<size_t, active_audio> &active_audio_,
                                             float category_multiplier, float delta_time)
        {
            for (auto it = active_audio_.begin(); it != active_audio_.end();)
            {
                auto &audio = it->second;
                if (!audio.m_is_fading)
                {
                    ++it;
                    continue;
                }

                audio.m_fade_elapsed += delta_time;
                bool finished = false;
                float current_volume;
                float diff = audio.m_fade_target_volume - audio.m_fade_start_volume;
                if (audio.m_fade_elapsed >= audio.m_fade_duration)
                {
                    current_volume = audio.m_fade_target_volume;
                    finished = true;
                }
                else
                    current_volume = audio.m_fade_start_volume + ((audio.m_fade_elapsed / audio.m_fade_duration) * diff);

                float min_volume = std::min(audio.m_fade_start_volume, audio.m_fade_target_volume);
                float max_volume = std::max(audio.m_fade_start_volume, audio.m_fade_target_volume);
                current_volume = std::clamp(current_volume, min_volume, max_volume);

                float final_volume = current_volume * m_master_volume * category_multiplier;

                auto source_allocation = m_resource_manager.get_source_allocation(audio.m_source_index);
                if (!(source_allocation.has_value()) || !(source_allocation.value().get().m_active))
                {
                    logcoe::warning("sound_manager::handle_fade_effects: handle " + std::to_string(it->first) + " is no longer active");
                    it = active_audio_.erase(it);
                    continue;
                }
                auto &source = source_allocation.value().get().m_source;
                if (!(source->set_volume(final_volume)))
                    logcoe::warning("sound_manager::handle_fade_effects: Failed to update the volume of handle " + std::to_string(it->first));

                if (finished)
                {
                    if (current_volume == 0.0f)
                    {
                        if (source->stop())
                        {
                            m_resource_manager.release_source(*source);
                            m_resource_manager.release_buffer(audio.m_filename);
                        }
                        else
                            logcoe::warning("sound_manager::handle_fade_effects: Failed to stop handle " + std::to_string(it->first) + " when finished to fade out");

                        it = active_audio_.erase(it);
                        continue;
                    }

                    audio.m_is_fading = false;
                    audio.m_base_volume = audio.m_fade_target_volume;
                    audio.m_fade_duration = 0.0f;
                    audio.m_fade_elapsed = 0.0f;
                    audio.m_fade_start_volume = 0.0f;
                    audio.m_fade_target_volume = 0.0f;
                }

                ++it;
            }
        }

        void sound_manager::handle_inactive_audio(std::unordered_map<size_t, active_audio> &active_audio_)
        {
            for (auto it = active_audio_.begin(); it != active_audio_.end();)
            {
                auto source_allocation = m_resource_manager.get_source_allocation(it->second.m_source_index);
                if (!(source_allocation.has_value()) || !(source_allocation.value().get().m_active))
                {
                    logcoe::debug("sound_manager::update: Cleaning up inactive audio handle: " + std::to_string(it->first));
                    it = active_audio_.erase(it);
                }
                else
                    ++it;
            }
        }

        sound_manager::sound_manager() : m_resource_manager(), m_next_sound_handle(1), m_next_music_handle(1),
                                       m_active_sounds(), m_active_music(), m_listener_position(),
                                       m_listener_velocity(), m_listener_forward(), m_listener_up(),
                                       m_last_update() { }

        sound_manager::~sound_manager() 
        {
            shutdown(); 
        }

        bool sound_manager::initialize(const std::string &audio_root_directory, size_t max_sources,
                                      size_t max_cache_size_mb, const std::string &sound_subdir,
                                      const std::string &music_subdir, LogLevel level)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (m_initialized)
            {
                logcoe::warning("sound_manager::initialize: Need to shutdown sound_manager before initialize it again");
                return false;
            }

            logcoe::initialize(level, "soundcoe");

            if (audio_root_directory.empty())
            {
                logcoe::error("sound_manager::initialize: Audio root directory cannot be empty");
                logcoe::shutdown();
                return false;
            }

            if (!std::filesystem::exists(audio_root_directory) || !std::filesystem::is_directory(audio_root_directory))
            {
                logcoe::error("sound_manager::initialize: Audio root directory does not exist or is not a directory: " + audio_root_directory);
                logcoe::shutdown();
                return false;
            }

            try
            {
                m_resource_manager.initialize(audio_root_directory, max_sources, max_cache_size_mb);
            }
            catch (const std::exception &e)
            {
                logcoe::error("sound_manager::initialize: Failed to create Resource Manager: " + std::string(e.what()));
                logcoe::shutdown();
                return false;
            }

            m_sound_subdir = sound_subdir + "/";
            m_music_subdir = music_subdir + "/";

            alListenerf(AL_GAIN, 1.0f);

            std::filesystem::path root_directory(audio_root_directory);
            std::filesystem::path general_audio_directory(root_directory / "general");
            if (std::filesystem::exists(general_audio_directory) && std::filesystem::is_directory(general_audio_directory))
            {
                if (!m_resource_manager.preload_directory("general"))
                {
                    logcoe::error("sound_manager::initialize: Failed to load general audio subdirectory");
                    m_resource_manager.shutdown();
                    logcoe::shutdown();
                    return false;
                }
            }
            else
                logcoe::warning("sound_manager::initialize: There is no general audio subdirectory");

            m_initialized = true;
            logcoe::info("sound_manager::initialize: sound_manager initialized successfully");
            return true;
        }

        void sound_manager::shutdown()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            logcoe::info("sound_manager::shutdown() called");

            m_next_sound_handle = 1;
            m_next_music_handle = 1;

            m_active_sounds.clear();
            m_active_music.clear();

            m_master_volume = 1.0f;
            m_master_sounds_volume = 1.0f;
            m_master_music_volume = 1.0f;
            m_master_pitch = 1.0f;
            m_master_sounds_pitch = 1.0f;
            m_master_music_pitch = 1.0f;

            m_mute = false;
            m_sounds_mute = false;
            m_music_mute = false;

            m_listener_position = vec3::zero();
            m_listener_velocity = vec3::zero();
            m_listener_forward = vec3::zero();
            m_listener_up = vec3::zero();

            m_last_update = std::chrono::steady_clock::time_point();
            m_first_update = true;

            m_last_error = "";
            m_has_error = false;

            m_resource_manager.shutdown();
            logcoe::info("sound_manager::shutdown() completed");
            logcoe::shutdown();
            m_initialized = false;
        }

        bool sound_manager::is_initialized() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            return m_initialized;
        }

        bool sound_manager::preload_scene(const std::string &scene_name)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_resource_manager.preload_directory(scene_name);
        }

        bool sound_manager::unload_scene(const std::string &scene_name)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_resource_manager.unload_directory(scene_name);
        }

        bool sound_manager::is_scene_loaded(const std::string &scene_name) const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_resource_manager.is_directory_loaded(scene_name);
        }

        void sound_manager::update()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto now = std::chrono::steady_clock::now();
            if (m_first_update)
            {
                m_last_update = now;
                m_first_update = false;
                return;
            }

            float delta_time = std::chrono::duration<float>(now - m_last_update).count();

            handle_streaming_audio();
            handle_fade_effects(m_active_sounds, m_master_sounds_volume, delta_time);
            handle_fade_effects(m_active_music, m_master_music_volume, delta_time);
            handle_inactive_audio(m_active_sounds);
            handle_inactive_audio(m_active_music);

            m_last_update = now;
        }

        sound_handle sound_manager::play_sound(const std::string &filename, float volume, float pitch, bool loop, sound_priority priority)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return play(m_active_sounds, m_sound_subdir + filename, volume, pitch, loop, priority, m_next_sound_handle, "play_sound",
                        m_master_sounds_volume, m_master_sounds_pitch);
        }

        sound_handle sound_manager::play_sound3d(const std::string &filename, const vec3 &position, const vec3 &velocity,
                                              float volume, float pitch, bool loop, sound_priority priority)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return play(m_active_sounds, m_sound_subdir + filename, volume, pitch, loop, priority, m_next_sound_handle, "play_sound3d",
                        m_master_sounds_volume, m_master_sounds_pitch, true, position, velocity);
        }

        music_handle sound_manager::play_music(const std::string &filename, float volume, float pitch, bool loop, sound_priority priority)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return play(m_active_music, m_music_subdir + filename, volume, pitch, loop, priority, m_next_music_handle, "play_music",
                        m_master_music_volume, m_master_music_pitch);
        }

        bool sound_manager::pause_sound(sound_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation(m_active_sounds, handle, sound_state::paused, "pause_sound");
        }

        bool sound_manager::pause_music(music_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation(m_active_music, handle, sound_state::paused, "pause_music");
        }

        bool sound_manager::pause_all_sounds()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation_all(m_active_sounds, sound_state::paused, "pause_all_sounds");
        }

        bool sound_manager::pause_all_music()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation_all(m_active_music, sound_state::paused, "pause_all_music");
        }

        bool sound_manager::pause_all()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation_all(m_active_sounds, sound_state::paused, "pause_all") &&
                   audio_operation_all(m_active_music, sound_state::paused, "pause_all");
        }

        bool sound_manager::resume_sound(sound_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (!check_audio_state(m_active_sounds, handle, sound_state::paused, "resume_sound"))
                return set_error("sound_manager::" + std::string("resume_sound") + ": Sound is not paused");

            return audio_operation(m_active_sounds, handle, sound_state::playing, "resume_sound");
        }

        bool sound_manager::resume_music(music_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (!check_audio_state(m_active_music, handle, sound_state::paused, "resume_music"))
                return set_error("sound_manager::" + std::string("resume_music") + ": Music is not paused");

            return audio_operation(m_active_music, handle, sound_state::playing, "resume_music");
        }

        bool sound_manager::resume_all_sounds()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation_all(m_active_sounds, sound_state::playing, "resume_all_sounds");
        }

        bool sound_manager::resume_all_music()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation_all(m_active_music, sound_state::playing, "resume_all_music");
        }

        bool sound_manager::resume_all()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation_all(m_active_sounds, sound_state::playing, "resume_all") &&
                   audio_operation_all(m_active_music, sound_state::playing, "resume_all");
        }

        bool sound_manager::stop_sound(sound_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation(m_active_sounds, handle, sound_state::stopped, "stop_sound");
        }

        bool sound_manager::stop_music(music_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation(m_active_music, handle, sound_state::stopped, "stop_music");
        }

        bool sound_manager::stop_all_sounds()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation_all(m_active_sounds, sound_state::stopped, "stop_all_sounds");
        }

        bool sound_manager::stop_all_music()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation_all(m_active_music, sound_state::stopped, "stop_all_music");
        }

        bool sound_manager::stop_all()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return audio_operation_all(m_active_sounds, sound_state::stopped, "stop_all") &&
                   audio_operation_all(m_active_music, sound_state::stopped, "stop_all");
        }

        bool sound_manager::set_sound_volume(sound_handle handle, float volume)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_audio_property(m_active_sounds, handle, property_type::volume, "set_sound_volume", volume);
        }

        bool sound_manager::set_music_volume(music_handle handle, float volume)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_audio_property(m_active_music, handle, property_type::volume, "set_music_volume", volume);
        }

        bool sound_manager::set_sound_pitch(sound_handle handle, float pitch)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_audio_property(m_active_sounds, handle, property_type::pitch, "set_sound_pitch", pitch);
        }

        bool sound_manager::set_music_pitch(music_handle handle, float pitch)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_audio_property(m_active_music, handle, property_type::pitch, "set_music_pitch", pitch);
        }

        bool sound_manager::set_sound_position(sound_handle handle, const vec3 &position)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_audio_property(m_active_sounds, handle, property_type::position, "set_sound_position",
                                    position.x, position.y, position.z);
        }

        bool sound_manager::set_sound_velocity(sound_handle handle, const vec3 &velocity)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_audio_property(m_active_sounds, handle, property_type::velocity, "set_sound_velocity",
                                    velocity.x, velocity.y, velocity.z);
        }

        bool sound_manager::is_sound_playing(sound_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return check_audio_state(m_active_sounds, handle, sound_state::playing, "is_sound_playing");
        }

        bool sound_manager::is_music_playing(music_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return check_audio_state(m_active_music, handle, sound_state::playing, "is_music_playing");
        }

        bool sound_manager::is_sound_paused(sound_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return check_audio_state(m_active_sounds, handle, sound_state::paused, "is_sound_paused");
        }

        bool sound_manager::is_music_paused(music_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return check_audio_state(m_active_music, handle, sound_state::paused, "is_music_paused");
        }

        bool sound_manager::is_sound_stopped(sound_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return check_audio_state(m_active_sounds, handle, sound_state::stopped, "is_sound_stopped");
        }

        bool sound_manager::is_music_stopped(music_handle handle)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return check_audio_state(m_active_music, handle, sound_state::stopped, "is_music_stopped");
        }

        size_t sound_manager::get_active_sounds_count() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_active_sounds.size();
        }

        size_t sound_manager::get_active_music_count() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_active_music.size();
        }

        sound_handle sound_manager::fade_in_sound(const std::string &filename, float duration,
                                              float volume, float pitch, bool loop, sound_priority priority)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            sound_handle handle = play(m_active_sounds, m_sound_subdir + filename, 0.0f, pitch, loop, priority, m_next_sound_handle, "fade_in_sound",
                                      m_master_sounds_volume, m_master_sounds_pitch);

            if (!is_handle_valid(handle))
                return INVALID_SOUND_HANDLE;

            active_audio &sound = m_active_sounds[handle];
            sound.m_base_volume = volume;

            if (fade(m_active_sounds, handle, true, duration, "fade_in_sound"))
                return handle;

            stop_sound(handle);
            return INVALID_SOUND_HANDLE;
        }

        music_handle sound_manager::fade_in_music(const std::string &filename, float duration,
                                              float volume, float pitch, bool loop, sound_priority priority)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            music_handle handle = play(m_active_music, m_music_subdir + filename, 0.0f, pitch, loop, priority, m_next_music_handle, "fade_in_music",
                                      m_master_music_volume, m_master_music_pitch);
            if (!is_handle_valid(handle))
                return INVALID_MUSIC_HANDLE;

            active_audio &music = m_active_music[handle];
            music.m_base_volume = volume;

            if (fade(m_active_music, handle, true, duration, "fade_in_music"))
                return handle;

            stop_music(handle);
            return INVALID_MUSIC_HANDLE;
        }

        bool sound_manager::fade_out_sound(sound_handle handle, float duration)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return fade(m_active_sounds, handle, false, duration, "fade_out_sound");
        }

        bool sound_manager::fade_out_music(music_handle handle, float duration)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return fade(m_active_music, handle, false, duration, "fade_out_music");
        }

        bool sound_manager::fade_to_volume_sound(sound_handle handle, float target_volume, float duration)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return fade_to_volume(m_active_sounds, handle, target_volume, duration, "fade_to_volume_sound");
        }

        bool sound_manager::fade_to_volume_music(music_handle handle, float target_volume, float duration)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return fade_to_volume(m_active_music, handle, target_volume, duration, "fade_to_volume_music");
        }

        bool sound_manager::set_master_volume(float volume)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_master_volume = volume;
            update_all_volume();

            return true;
        }

        bool sound_manager::set_master_sounds_volume(float volume)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_master_sounds_volume = volume;
            update_all_sounds_volume();

            return true;
        }

        bool sound_manager::set_master_music_volume(float volume)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_master_music_volume = volume;
            update_all_music_volume();

            return true;
        }

        bool sound_manager::set_master_pitch(float pitch)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_master_pitch = pitch;
            update_all_pitch();

            return true;
        }

        bool sound_manager::set_master_sounds_pitch(float pitch)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_master_sounds_pitch = pitch;
            update_all_sounds_pitch();

            return true;
        }

        bool sound_manager::set_master_music_pitch(float pitch)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_master_music_pitch = pitch;
            update_all_music_pitch();

            return true;
        }

        float sound_manager::get_master_volume() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_master_volume;
        }

        float sound_manager::get_master_sounds_volume() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_master_sounds_volume;
        }

        float sound_manager::get_master_music_volume() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_master_music_volume;
        }

        float sound_manager::get_master_pitch() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_master_pitch;
        }

        float sound_manager::get_master_sounds_pitch() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_master_sounds_pitch;
        }

        float sound_manager::get_master_music_pitch() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_master_music_pitch;
        }

        bool sound_manager::mute_all_sounds()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_sounds_mute = true;
            update_all_sounds_volume();

            return true;
        }

        bool sound_manager::mute_all_music()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_music_mute = true;
            update_all_music_volume();

            return true;
        }

        bool sound_manager::mute_all()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_mute = true;
            update_all_volume();

            return true;
        }

        bool sound_manager::unmute_all_sounds()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_sounds_mute = false;
            update_all_sounds_volume();

            return true;
        }

        bool sound_manager::unmute_all_music()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_music_mute = false;
            update_all_music_volume();

            return true;
        }

        bool sound_manager::unmute_all()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_mute = m_sounds_mute = m_music_mute = false;

            update_all_volume();

            return true;
        }

        bool sound_manager::is_muted() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_mute;
        }

        bool sound_manager::is_sounds_muted() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_sounds_mute;
        }

        bool sound_manager::is_music_muted() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_music_mute;
        }

        bool sound_manager::update_listener(const vec3 &position, const vec3 &velocity, const vec3 &forward, const vec3 &up)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_listener_position_impl(position) &&
                   set_listener_velocity_impl(velocity) &&
                   set_listener_orientation_impl(forward, up);
        }

        bool sound_manager::set_listener_position(const vec3 &position)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_listener_position_impl(position);
        }

        bool sound_manager::set_listener_velocity(const vec3 &velocity)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_listener_velocity_impl(velocity);
        }

        bool sound_manager::set_listener_forward(const vec3 &forward)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_listener_orientation_impl(forward, m_listener_up);
        }

        bool sound_manager::set_listener_up(const vec3 &up)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return set_listener_orientation_impl(m_listener_forward, up);
        }

        vec3 sound_manager::get_listener_position()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_listener_position;
        }

        vec3 sound_manager::get_listener_velocity()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_listener_velocity;
        }

        vec3 sound_manager::get_listener_forward()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_listener_forward;
        }

        vec3 sound_manager::get_listener_up()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            return m_listener_up;
        }

        const std::string sound_manager::get_error()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::string error(m_last_error);
            m_last_error = "";
            m_has_error = false;
            return error;
        }

        void sound_manager::clear_error()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_last_error = "";
            m_has_error = false;
        }

        bool sound_manager::is_handle_valid(size_t handle) { return handle != INVALID_SOUND_HANDLE; }
    } // namespace detail
} // namespace soundcoe
