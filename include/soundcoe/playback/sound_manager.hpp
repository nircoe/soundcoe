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

            SoundHandle playSound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                SoundPriority priority = SoundPriority::Medium);
            SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity = Vec3::zero(),
                                    float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                    SoundPriority priority = SoundPriority::Medium);
            MusicHandle playMusic(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                SoundPriority priority = SoundPriority::Critical);

            bool pauseSound(SoundHandle handle);
            bool pauseMusic(MusicHandle handle);
            bool pauseAllSounds();
            bool pauseAllMusic();
            bool pauseAll();
            bool resumeSound(SoundHandle handle);
            bool resumeMusic(MusicHandle handle);
            bool resumeAllSounds();
            bool resumeAllMusic();
            bool resumeAll();
            bool stopSound(SoundHandle handle);
            bool stopMusic(MusicHandle handle);
            bool stopAllSounds();
            bool stopAllMusic();
            bool stopAll();
            bool setSoundVolume(SoundHandle handle, float volume);
            bool setMusicVolume(MusicHandle handle, float volume);
            bool setSoundPitch(SoundHandle handle, float pitch);
            bool setMusicPitch(MusicHandle handle, float pitch);
            bool setSoundPosition(SoundHandle handle, const Vec3 &position);
            bool setSoundVelocity(SoundHandle handle, const Vec3 &velocity);

            bool isSoundPlaying(SoundHandle handle);
            bool isMusicPlaying(MusicHandle handle);
            bool isSoundPaused(SoundHandle handle);
            bool isMusicPaused(MusicHandle handle);
            bool isSoundStopped(SoundHandle handle);
            bool isMusicStopped(MusicHandle handle);

            size_t getActiveSoundsCount() const;
            size_t getActiveMusicCount() const;

            SoundHandle fadeInSound(const std::string &filename, float duration,
                                    float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                    SoundPriority priority = SoundPriority::Medium);
            MusicHandle fadeInMusic(const std::string &filename, float duration,
                                    float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                    SoundPriority priority = SoundPriority::Critical);
            bool fadeOutSound(SoundHandle handle, float duration);
            bool fadeOutMusic(MusicHandle handle, float duration);
            bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration);
            bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration);

            bool setMasterVolume(float volume);
            bool setMasterSoundsVolume(float volume);
            bool setMasterMusicVolume(float volume);
            bool setMasterPitch(float pitch);
            bool setMasterSoundsPitch(float pitch);
            bool setMasterMusicPitch(float pitch);
            float getMasterVolume() const;
            float getMasterSoundsVolume() const;
            float getMasterMusicVolume() const;
            float getMasterPitch() const;
            float getMasterSoundsPitch() const;
            float getMasterMusicPitch() const;

            bool muteAllSounds();
            bool muteAllMusic();
            bool muteAll();
            bool unmuteAllSounds();
            bool unmuteAllMusic();
            bool unmuteAll();
            bool isMuted() const;
            bool isSoundsMuted() const;
            bool isMusicMuted() const;

            bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up = Vec3::up());
            bool setListenerPosition(const Vec3 &position);
            bool setListenerVelocity(const Vec3 &velocity);
            bool setListenerForward(const Vec3 &forward);
            bool setListenerUp(const Vec3 &up = Vec3::up());

            Vec3 getListenerPosition();
            Vec3 getListenerVelocity();
            Vec3 getListenerForward();
            Vec3 getListenerUp();

            const std::string getError();
            void clearError();

            static bool is_handle_valid(size_t handle);
        };
    } // namespace detail
} // namespace soundcoe
