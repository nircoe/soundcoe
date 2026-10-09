#include <soundcoe.hpp>
#include <soundcoe/playback/sound_manager.hpp>
#include <cassert>
#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace internal
    {
        sound_manager& get_sound_manager_instance()
        {
            static sound_manager s_sound_manager;
            return s_sound_manager;
        }
    }

    std::expected<void, error> initialize(const std::string &audio_root_directory, size_t max_sources,
                                          size_t max_cache_size_mb, const std::string &sound_subdir,
                                          const std::string &music_subdir, LogLevel level)
    {
        return internal::get_sound_manager_instance().initialize(audio_root_directory, max_sources, max_cache_size_mb,
                                                            sound_subdir, music_subdir, level);
    }

    std::expected<void, error> initialize(const init_config &config)
    {
        return initialize(config.audio_root_directory, config.max_sources, config.max_cache_size_mb,
                          config.sound_subdir, config.music_subdir, config.level);
    }

    void shutdown()
    {
        internal::get_sound_manager_instance().shutdown();
    }

    bool is_initialized()
    {
        return internal::get_sound_manager_instance().is_initialized();
    }

    std::expected<void, error> preload_scene(const std::string &scene_name)
    {
        return internal::get_sound_manager_instance().preload_scene(scene_name);
    }

    std::expected<void, error> unload_scene(const std::string &scene_name)
    {
        return internal::get_sound_manager_instance().unload_scene(scene_name);
    }

    bool is_scene_loaded(const std::string &scene_name)
    {
        return internal::get_sound_manager_instance().is_scene_loaded(scene_name);
    }

    void update()
    {
        internal::get_sound_manager_instance().update();
    }

    std::expected<sound_handle, error> play_sound(const std::string &filename, float volume, float pitch, bool loop,
                                                  sound_priority priority)
    {
        return internal::get_sound_manager_instance().play_sound(filename, volume, pitch, loop, priority);
    }

    std::expected<sound_handle, error> play_sound3d(const std::string &filename, const vec3 &position,
                                                    const vec3 &velocity, float volume, float pitch, bool loop,
                                                    sound_priority priority)
    {
        return internal::get_sound_manager_instance().play_sound3d(filename, position, velocity, volume, pitch, loop, priority);
    }

    std::expected<music_handle, error> play_music(const std::string &filename, float volume, float pitch, bool loop,
                                                  sound_priority priority)
    {
        return internal::get_sound_manager_instance().play_music(filename, volume, pitch, loop, priority);
    }

    std::expected<void, error> pause_sound(sound_handle handle)
    {
        return internal::get_sound_manager_instance().pause_sound(handle);
    }

    std::expected<void, error> pause_music(music_handle handle)
    {
        return internal::get_sound_manager_instance().pause_music(handle);
    }

    void pause_all_sounds()
    {
        [[maybe_unused]] auto r = internal::get_sound_manager_instance().pause_all_sounds();
        assert(r);
    }

    void pause_all_music()
    {
        [[maybe_unused]] auto r = internal::get_sound_manager_instance().pause_all_music();
        assert(r);
    }

    void pause_all()
    {
        [[maybe_unused]] auto r = internal::get_sound_manager_instance().pause_all();
        assert(r);
    }

    std::expected<void, error> resume_sound(sound_handle handle)
    {
        return internal::get_sound_manager_instance().resume_sound(handle);
    }

    std::expected<void, error> resume_music(music_handle handle)
    {
        return internal::get_sound_manager_instance().resume_music(handle);
    }

    void resume_all_sounds()
    {
        [[maybe_unused]] auto r = internal::get_sound_manager_instance().resume_all_sounds();
        assert(r);
    }

    void resume_all_music()
    {
        [[maybe_unused]] auto r = internal::get_sound_manager_instance().resume_all_music();
        assert(r);
    }

    void resume_all()
    {
        [[maybe_unused]] auto r = internal::get_sound_manager_instance().resume_all();
        assert(r);
    }

    std::expected<void, error> stop_sound(sound_handle handle)
    {
        return internal::get_sound_manager_instance().stop_sound(handle);
    }

    std::expected<void, error> stop_music(music_handle handle)
    {
        return internal::get_sound_manager_instance().stop_music(handle);
    }

    void stop_all_sounds()
    {
        [[maybe_unused]] auto r = internal::get_sound_manager_instance().stop_all_sounds();
        assert(r);
    }

    void stop_all_music()
    {
        [[maybe_unused]] auto r = internal::get_sound_manager_instance().stop_all_music();
        assert(r);
    }

    void stop_all()
    {
        [[maybe_unused]] auto r = internal::get_sound_manager_instance().stop_all();
        assert(r);
    }

    std::expected<void, error> set_sound_volume(sound_handle handle, float volume)
    {
        return internal::get_sound_manager_instance().set_sound_volume(handle, volume);
    }

    std::expected<void, error> set_music_volume(music_handle handle, float volume)
    {
        return internal::get_sound_manager_instance().set_music_volume(handle, volume);
    }

    std::expected<void, error> set_sound_pitch(sound_handle handle, float pitch)
    {
        return internal::get_sound_manager_instance().set_sound_pitch(handle, pitch);
    }

    std::expected<void, error> set_music_pitch(music_handle handle, float pitch)
    {
        return internal::get_sound_manager_instance().set_music_pitch(handle, pitch);
    }

    std::expected<void, error> set_sound_position(sound_handle handle, const vec3 &position)
    {
        return internal::get_sound_manager_instance().set_sound_position(handle, position);
    }

    std::expected<void, error> set_sound_velocity(sound_handle handle, const vec3 &velocity)
    {
        return internal::get_sound_manager_instance().set_sound_velocity(handle, velocity);
    }

    std::expected<bool, error> is_sound_playing(sound_handle handle)
    {
        return internal::get_sound_manager_instance().is_sound_playing(handle);
    }

    std::expected<bool, error> is_music_playing(music_handle handle)
    {
        return internal::get_sound_manager_instance().is_music_playing(handle);
    }

    std::expected<bool, error> is_sound_paused(sound_handle handle)
    {
        return internal::get_sound_manager_instance().is_sound_paused(handle);
    }

    std::expected<bool, error> is_music_paused(music_handle handle)
    {
        return internal::get_sound_manager_instance().is_music_paused(handle);
    }

    std::expected<bool, error> is_sound_stopped(sound_handle handle)
    {
        return internal::get_sound_manager_instance().is_sound_stopped(handle);
    }

    std::expected<bool, error> is_music_stopped(music_handle handle)
    {
        return internal::get_sound_manager_instance().is_music_stopped(handle);
    }

    size_t get_active_sounds_count()
    {
        return internal::get_sound_manager_instance().get_active_sounds_count();
    }

    size_t get_active_music_count()
    {
        return internal::get_sound_manager_instance().get_active_music_count();
    }

    std::expected<sound_handle, error> fade_in_sound(const std::string &filename, float duration, float volume,
                                                     float pitch, bool loop, sound_priority priority)
    {
        return internal::get_sound_manager_instance().fade_in_sound(filename, duration, volume, pitch, loop, priority);
    }

    std::expected<music_handle, error> fade_in_music(const std::string &filename, float duration, float volume,
                                                     float pitch, bool loop, sound_priority priority)
    {
        return internal::get_sound_manager_instance().fade_in_music(filename, duration, volume, pitch, loop, priority);
    }

    std::expected<void, error> fade_out_sound(sound_handle handle, float duration)
    {
        return internal::get_sound_manager_instance().fade_out_sound(handle, duration);
    }

    std::expected<void, error> fade_out_music(music_handle handle, float duration)
    {
        return internal::get_sound_manager_instance().fade_out_music(handle, duration);
    }

    std::expected<void, error> fade_to_volume_sound(sound_handle handle, float target_volume, float duration)
    {
        return internal::get_sound_manager_instance().fade_to_volume_sound(handle, target_volume, duration);
    }

    std::expected<void, error> fade_to_volume_music(music_handle handle, float target_volume, float duration)
    {
        return internal::get_sound_manager_instance().fade_to_volume_music(handle, target_volume, duration);
    }

    void set_master_volume(float volume)
    {
        internal::get_sound_manager_instance().set_master_volume(volume);
    }

    void set_master_sounds_volume(float volume)
    {
        internal::get_sound_manager_instance().set_master_sounds_volume(volume);
    }

    void set_master_music_volume(float volume)
    {
        internal::get_sound_manager_instance().set_master_music_volume(volume);
    }

    void set_master_pitch(float pitch)
    {
        internal::get_sound_manager_instance().set_master_pitch(pitch);
    }

    void set_master_sounds_pitch(float pitch)
    {
        internal::get_sound_manager_instance().set_master_sounds_pitch(pitch);
    }

    void set_master_music_pitch(float pitch)
    {
        internal::get_sound_manager_instance().set_master_music_pitch(pitch);
    }

    float get_master_volume()
    {
        return internal::get_sound_manager_instance().get_master_volume();
    }

    float get_master_sounds_volume()
    {
        return internal::get_sound_manager_instance().get_master_sounds_volume();
    }

    float get_master_music_volume()
    {
        return internal::get_sound_manager_instance().get_master_music_volume();
    }

    float get_master_pitch()
    {
        return internal::get_sound_manager_instance().get_master_pitch();
    }

    float get_master_sounds_pitch()
    {
        return internal::get_sound_manager_instance().get_master_sounds_pitch();
    }

    float get_master_music_pitch()
    {
        return internal::get_sound_manager_instance().get_master_music_pitch();
    }

    void mute_all_sounds()
    {
        internal::get_sound_manager_instance().mute_all_sounds();
    }

    void mute_all_music()
    {
        internal::get_sound_manager_instance().mute_all_music();
    }

    void mute_all()
    {
        internal::get_sound_manager_instance().mute_all();
    }

    void unmute_all_sounds()
    {
        internal::get_sound_manager_instance().unmute_all_sounds();
    }

    void unmute_all_music()
    {
        internal::get_sound_manager_instance().unmute_all_music();
    }

    void unmute_all()
    {
        internal::get_sound_manager_instance().unmute_all();
    }

    bool is_muted()
    {
        return internal::get_sound_manager_instance().is_muted();
    }

    bool is_sounds_muted()
    {
        return internal::get_sound_manager_instance().is_sounds_muted();
    }

    bool is_music_muted()
    {
        return internal::get_sound_manager_instance().is_music_muted();
    }

    std::expected<void, error> update_listener(const vec3 &position, const vec3 &velocity, const vec3 &forward,
                                               const vec3 &up)
    {
        return internal::get_sound_manager_instance().update_listener(position, velocity, forward, up);
    }

    std::expected<void, error> set_listener_position(const vec3 &position)
    {
        return internal::get_sound_manager_instance().set_listener_position(position);
    }

    std::expected<void, error> set_listener_velocity(const vec3 &velocity)
    {
        return internal::get_sound_manager_instance().set_listener_velocity(velocity);
    }

    std::expected<void, error> set_listener_forward(const vec3 &forward)
    {
        return internal::get_sound_manager_instance().set_listener_forward(forward);
    }

    std::expected<void, error> set_listener_up(const vec3 &up)
    {
        return internal::get_sound_manager_instance().set_listener_up(up);
    }

    vec3 get_listener_position()
    {
        return internal::get_sound_manager_instance().get_listener_position();
    }

    vec3 get_listener_velocity()
    {
        return internal::get_sound_manager_instance().get_listener_velocity();
    }

    vec3 get_listener_forward()
    {
        return internal::get_sound_manager_instance().get_listener_forward();
    }

    vec3 get_listener_up()
    {
        return internal::get_sound_manager_instance().get_listener_up();
    }

    bool is_handle_valid(size_t handle)
    {
        return internal::get_sound_manager_instance().is_handle_valid(handle);
    }
} // namespace soundcoe
