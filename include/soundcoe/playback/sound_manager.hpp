#pragma once

#include <soundcoe/resources/resource_manager.hpp>
#include <soundcoe/core/types.hpp>
#include <string>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <chrono>

namespace soundcoe
{
    constexpr sound_handle INVALID_SOUND_HANDLE = 0;
    constexpr music_handle INVALID_MUSIC_HANDLE = 0;

    namespace detail
    {
        struct active_audio
        {
            size_t m_source_index;
            std::string m_filename;
            float m_base_volume;
            float m_base_pitch;
            bool m_loop;

            bool m_stream = false;
            size_t m_stream_buffer_size = 0;
            float m_stream_position = 0.0f;
            bool m_stream_needs_refill = false;

            bool m_is_fading = false;
            float m_fade_duration = 0.0f;
            float m_fade_elapsed = 0.0f;
            float m_fade_start_volume = 0.0f;
            float m_fade_target_volume = 0.0f;
        };

        class sound_manager
        {
            bool m_initialized = false;

            resource_manager m_resource_manager;
            std::string m_sound_subdir;
            std::string m_music_subdir;

            std::atomic<size_t> m_next_sound_handle;
            std::atomic<size_t> m_next_music_handle;

            mutable std::mutex m_mutex;
            std::unordered_map<sound_handle, active_audio> m_active_sounds;
            std::unordered_map<music_handle, active_audio> m_active_music;

            float m_master_volume = 1.0f;
            float m_master_sounds_volume = 1.0f;
            float m_master_music_volume = 1.0f;
            float m_master_pitch = 1.0f;
            float m_master_sounds_pitch = 1.0f;
            float m_master_music_pitch = 1.0f;

            bool m_mute = false;
            bool m_sounds_mute = false;
            bool m_music_mute = false;

            vec3 m_listener_position;
            vec3 m_listener_velocity;
            vec3 m_listener_forward;
            vec3 m_listener_up;

            std::chrono::steady_clock::time_point m_last_update;
            bool m_first_update = true;

            std::string m_last_error = "";
            bool m_has_error = false;

            template <typename Setter, typename Getter>
            void update_all_audio_property(std::unordered_map<size_t, active_audio> &active_audio_,
                                        Setter set_property, Getter get_base_property,
                                        float master_multiplier, float category_multiplier)
            {
                for (auto it = active_audio_.begin(); it != active_audio_.end();)
                {
                    auto source_allocation = m_resource_manager.get_source_allocation(it->second.m_source_index);
                    if (!(source_allocation.has_value()) || !(source_allocation.value().get().m_active))
                    {
                        it = active_audio_.erase(it);
                        continue;
                    }

                    float final_value = get_base_property(it->second) * master_multiplier * category_multiplier;
                    set_property(source_allocation.value().get().m_source, final_value);
                    ++it;
                }
            }

            void update_all_sounds_volume();
            void update_all_music_volume();
            void update_all_volume();
            void update_all_sounds_pitch();
            void update_all_music_pitch();
            void update_all_pitch();

            bool set_listener_position_impl(const vec3 &position);
            bool set_listener_velocity_impl(const vec3 &velocity);
            bool set_listener_orientation_impl(const vec3 &forward, const vec3 &up);

            bool set_error(const std::string &error);

            bool fade_to_volume(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                            float target_volume, float duration, const std::string &method);
            bool fade(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                    bool fade_in, float duration, const std::string &method);

            bool check_audio_state(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                                sound_state state, const std::string &method);

            bool set_audio_property(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                                property_type type, const std::string &method,
                                float value, float y = 0.0f, float z = 0.0f);

            bool audio_operation(std::unordered_map<size_t, active_audio> &active_audio_, size_t handle,
                                sound_state operation, const std::string &method);
            bool audio_operation_all(std::unordered_map<size_t, active_audio> &active_audio_, sound_state operation,
                                const std::string &method);

            size_t play(std::unordered_map<size_t, active_audio> &active_audio_, const std::string &filename,
                        float volume, float pitch, bool loop, sound_priority priority,
                        std::atomic<size_t> &next_handle, const std::string &method,
                        float master_category_volume, float master_category_pitch,
                        bool is_3d = false, const vec3 &position = vec3::zero(), const vec3 &velocity = vec3::zero());

            void handle_streaming_audio();
            void handle_fade_effects(std::unordered_map<size_t, active_audio> &active_audio_,
                                float category_multiplier, float delta_time);
            void handle_inactive_audio(std::unordered_map<size_t, active_audio> &active_audio_);

        public:
            sound_manager();
            ~sound_manager();

            bool initialize(const std::string &audio_root_directory, size_t max_sources = 64,
                            size_t max_cache_size_mb = UNLIMITED_CACHE, const std::string &sound_subdir = "sfx",
                            const std::string &music_subdir = "music", LogLevel level = LogLevel::DEBUG);
            void shutdown();
            bool is_initialized() const;

            bool preload_scene(const std::string &scene_name);
            bool unload_scene(const std::string &scene_name);
            bool is_scene_loaded(const std::string &scene_name) const;

            void update();

            sound_handle play_sound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                sound_priority priority = sound_priority::medium);
            sound_handle play_sound3d(const std::string &filename, const vec3 &position, const vec3 &velocity = vec3::zero(),
                                    float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                    sound_priority priority = sound_priority::medium);
            music_handle play_music(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                sound_priority priority = sound_priority::critical);

            bool pause_sound(sound_handle handle);
            bool pause_music(music_handle handle);
            bool pause_all_sounds();
            bool pause_all_music();
            bool pause_all();
            bool resume_sound(sound_handle handle);
            bool resume_music(music_handle handle);
            bool resume_all_sounds();
            bool resume_all_music();
            bool resume_all();
            bool stop_sound(sound_handle handle);
            bool stop_music(music_handle handle);
            bool stop_all_sounds();
            bool stop_all_music();
            bool stop_all();
            bool set_sound_volume(sound_handle handle, float volume);
            bool set_music_volume(music_handle handle, float volume);
            bool set_sound_pitch(sound_handle handle, float pitch);
            bool set_music_pitch(music_handle handle, float pitch);
            bool set_sound_position(sound_handle handle, const vec3 &position);
            bool set_sound_velocity(sound_handle handle, const vec3 &velocity);

            bool is_sound_playing(sound_handle handle);
            bool is_music_playing(music_handle handle);
            bool is_sound_paused(sound_handle handle);
            bool is_music_paused(music_handle handle);
            bool is_sound_stopped(sound_handle handle);
            bool is_music_stopped(music_handle handle);

            size_t get_active_sounds_count() const;
            size_t get_active_music_count() const;

            sound_handle fade_in_sound(const std::string &filename, float duration,
                                    float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                    sound_priority priority = sound_priority::medium);
            music_handle fade_in_music(const std::string &filename, float duration,
                                    float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                    sound_priority priority = sound_priority::critical);
            bool fade_out_sound(sound_handle handle, float duration);
            bool fade_out_music(music_handle handle, float duration);
            bool fade_to_volume_sound(sound_handle handle, float target_volume, float duration);
            bool fade_to_volume_music(music_handle handle, float target_volume, float duration);

            bool set_master_volume(float volume);
            bool set_master_sounds_volume(float volume);
            bool set_master_music_volume(float volume);
            bool set_master_pitch(float pitch);
            bool set_master_sounds_pitch(float pitch);
            bool set_master_music_pitch(float pitch);
            float get_master_volume() const;
            float get_master_sounds_volume() const;
            float get_master_music_volume() const;
            float get_master_pitch() const;
            float get_master_sounds_pitch() const;
            float get_master_music_pitch() const;

            bool mute_all_sounds();
            bool mute_all_music();
            bool mute_all();
            bool unmute_all_sounds();
            bool unmute_all_music();
            bool unmute_all();
            bool is_muted() const;
            bool is_sounds_muted() const;
            bool is_music_muted() const;

            bool update_listener(const vec3 &position, const vec3 &velocity, const vec3 &forward, const vec3 &up = vec3::up());
            bool set_listener_position(const vec3 &position);
            bool set_listener_velocity(const vec3 &velocity);
            bool set_listener_forward(const vec3 &forward);
            bool set_listener_up(const vec3 &up = vec3::up());

            vec3 get_listener_position();
            vec3 get_listener_velocity();
            vec3 get_listener_forward();
            vec3 get_listener_up();

            const std::string get_error();
            void clear_error();

            static bool is_handle_valid(size_t handle);
        };
    } // namespace detail
} // namespace soundcoe
