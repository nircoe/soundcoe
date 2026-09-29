#include <soundcoe.hpp>
#include <soundcoe/playback/sound_manager.hpp>
#include <cassert>
#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace detail
    {
        sound_manager& get_sound_manager_instance()
        {
            static sound_manager s_sound_manager;
            return s_sound_manager;
        }
    }

    bool initialize(const std::string &audio_root_directory, size_t max_sources,
                    size_t max_cache_size_mb, const std::string &sound_subdir,
                    const std::string &music_subdir, LogLevel level)
    {
        return detail::get_sound_manager_instance().initialize(audio_root_directory, max_sources, max_cache_size_mb,
                                                            sound_subdir, music_subdir, level);
    }

    bool initialize(const init_config &config)
    {
        return initialize(config.audio_root_directory, config.max_sources, config.max_cache_size_mb,
                          config.sound_subdir, config.music_subdir, config.level);
    }

    void shutdown()
    {
        detail::get_sound_manager_instance().shutdown();
    }

    bool is_initialized()
    {
        return detail::get_sound_manager_instance().is_initialized();
    }

    bool preload_scene(const std::string &scene_name)
    {
        return detail::get_sound_manager_instance().preload_scene(scene_name);
    }

    bool unload_scene(const std::string &scene_name)
    {
        return detail::get_sound_manager_instance().unload_scene(scene_name);
    }

    bool is_scene_loaded(const std::string &scene_name)
    {
        return detail::get_sound_manager_instance().is_scene_loaded(scene_name);
    }

    void update()
    {
        detail::get_sound_manager_instance().update();
    }

    sound_handle play_sound(const std::string &filename, float volume, float pitch, bool loop,
                          sound_priority priority)
    {
        return detail::get_sound_manager_instance().play_sound(filename, volume, pitch, loop, priority);
    }

    sound_handle play_sound3d(const std::string &filename, const vec3 &position, const vec3 &velocity,
                            float volume, float pitch, bool loop,
                            sound_priority priority)
    {
        return detail::get_sound_manager_instance().play_sound3d(filename, position, velocity, volume, pitch, loop, priority);
    }

    music_handle play_music(const std::string &filename, float volume, float pitch, bool loop,
                          sound_priority priority)
    {
        return detail::get_sound_manager_instance().play_music(filename, volume, pitch, loop, priority);
    }

    bool pause_sound(sound_handle handle)
    {
        return detail::get_sound_manager_instance().pause_sound(handle);
    }

    bool pause_music(music_handle handle)
    {
        return detail::get_sound_manager_instance().pause_music(handle);
    }

    void pause_all_sounds()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().pause_all_sounds();
        assert(succeed);
    }

    void pause_all_music()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().pause_all_music();
        assert(succeed);
    }

    void pause_all()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().pause_all();
        assert(succeed);
    }

    bool resume_sound(sound_handle handle)
    {
        return detail::get_sound_manager_instance().resume_sound(handle);
    }

    bool resume_music(music_handle handle)
    {
        return detail::get_sound_manager_instance().resume_music(handle);
    }

    void resume_all_sounds()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().resume_all_sounds();
        assert(succeed);
    }

    void resume_all_music()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().resume_all_music();
        assert(succeed);
    }

    void resume_all()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().resume_all();
        assert(succeed);
    }

    bool stop_sound(sound_handle handle)
    {
        return detail::get_sound_manager_instance().stop_sound(handle);
    }

    bool stop_music(music_handle handle)
    {
        return detail::get_sound_manager_instance().stop_music(handle);
    }

    void stop_all_sounds()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().stop_all_sounds();
        assert(succeed);
    }

    void stop_all_music()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().stop_all_music();
        assert(succeed);
    }

    void stop_all()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().stop_all();
        assert(succeed);
    }

    bool set_sound_volume(sound_handle handle, float volume)
    {
        return detail::get_sound_manager_instance().set_sound_volume(handle, volume);
    }

    bool set_music_volume(music_handle handle, float volume)
    {
        return detail::get_sound_manager_instance().set_music_volume(handle, volume);
    }

    bool set_sound_pitch(sound_handle handle, float pitch)
    {
        return detail::get_sound_manager_instance().set_sound_pitch(handle, pitch);
    }

    bool set_music_pitch(music_handle handle, float pitch)
    {
        return detail::get_sound_manager_instance().set_music_pitch(handle, pitch);
    }

    bool set_sound_position(sound_handle handle, const vec3 &position)
    {
        return detail::get_sound_manager_instance().set_sound_position(handle, position);
    }

    bool set_sound_velocity(sound_handle handle, const vec3 &velocity)
    {
        return detail::get_sound_manager_instance().set_sound_velocity(handle, velocity);
    }

    bool is_sound_playing(sound_handle handle)
    {
        return detail::get_sound_manager_instance().is_sound_playing(handle);
    }

    bool is_music_playing(music_handle handle)
    {
        return detail::get_sound_manager_instance().is_music_playing(handle);
    }

    bool is_sound_paused(sound_handle handle)
    {
        return detail::get_sound_manager_instance().is_sound_paused(handle);
    }

    bool is_music_paused(music_handle handle)
    {
        return detail::get_sound_manager_instance().is_music_paused(handle);
    }

    bool is_sound_stopped(sound_handle handle)
    {
        return detail::get_sound_manager_instance().is_sound_stopped(handle);
    }

    bool is_music_stopped(music_handle handle)
    {
        return detail::get_sound_manager_instance().is_music_stopped(handle);
    }

    size_t get_active_sounds_count()
    {
        return detail::get_sound_manager_instance().get_active_sounds_count();
    }

    size_t get_active_music_count()
    {
        return detail::get_sound_manager_instance().get_active_music_count();
    }

    sound_handle fade_in_sound(const std::string &filename, float duration,
                            float volume, float pitch, bool loop,
                            sound_priority priority)
    {
        return detail::get_sound_manager_instance().fade_in_sound(filename, duration, volume, pitch, loop, priority);
    }

    music_handle fade_in_music(const std::string &filename, float duration,
                            float volume, float pitch, bool loop,
                            sound_priority priority)
    {
        return detail::get_sound_manager_instance().fade_in_music(filename, duration, volume, pitch, loop, priority);
    }

    bool fade_out_sound(sound_handle handle, float duration)
    {
        return detail::get_sound_manager_instance().fade_out_sound(handle, duration);
    }

    bool fade_out_music(music_handle handle, float duration)
    {
        return detail::get_sound_manager_instance().fade_out_music(handle, duration);
    }

    bool fade_to_volume_sound(sound_handle handle, float target_volume, float duration)
    {
        return detail::get_sound_manager_instance().fade_to_volume_sound(handle, target_volume, duration);
    }

    bool fade_to_volume_music(music_handle handle, float target_volume, float duration)
    {
        return detail::get_sound_manager_instance().fade_to_volume_music(handle, target_volume, duration);
    }

    bool set_master_volume(float volume)
    {
        return detail::get_sound_manager_instance().set_master_volume(volume);
    }

    bool set_master_sounds_volume(float volume)
    {
        return detail::get_sound_manager_instance().set_master_sounds_volume(volume);
    }

    bool set_master_music_volume(float volume)
    {
        return detail::get_sound_manager_instance().set_master_music_volume(volume);
    }

    bool set_master_pitch(float pitch)
    {
        return detail::get_sound_manager_instance().set_master_pitch(pitch);
    }

    bool set_master_sounds_pitch(float pitch)
    {
        return detail::get_sound_manager_instance().set_master_sounds_pitch(pitch);
    }

    bool set_master_music_pitch(float pitch)
    {
        return detail::get_sound_manager_instance().set_master_music_pitch(pitch);
    }

    float get_master_volume()
    {
        return detail::get_sound_manager_instance().get_master_volume();
    }

    float get_master_sounds_volume()
    {
        return detail::get_sound_manager_instance().get_master_sounds_volume();
    }

    float get_master_music_volume()
    {
        return detail::get_sound_manager_instance().get_master_music_volume();
    }

    float get_master_pitch()
    {
        return detail::get_sound_manager_instance().get_master_pitch();
    }

    float get_master_sounds_pitch()
    {
        return detail::get_sound_manager_instance().get_master_sounds_pitch();
    }

    float get_master_music_pitch()
    {
        return detail::get_sound_manager_instance().get_master_music_pitch();
    }

    void mute_all_sounds()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().mute_all_sounds();
        assert(succeed);
    }

    void mute_all_music()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().mute_all_music();
        assert(succeed);
    }

    void mute_all()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().mute_all();
        assert(succeed);
    }

    void unmute_all_sounds()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().unmute_all_sounds();
        assert(succeed);
    }

    void unmute_all_music()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().unmute_all_music();
        assert(succeed);
    }

    void unmute_all()
    {
        [[maybe_unused]] bool succeed = detail::get_sound_manager_instance().unmute_all();
        assert(succeed);
    }

    bool is_muted()
    {
        return detail::get_sound_manager_instance().is_muted();
    }

    bool is_sounds_muted()
    {
        return detail::get_sound_manager_instance().is_sounds_muted();
    }

    bool is_music_muted()
    {
        return detail::get_sound_manager_instance().is_music_muted();
    }

    bool update_listener(const vec3 &position, const vec3 &velocity, const vec3 &forward, const vec3 &up)
    {
        return detail::get_sound_manager_instance().update_listener(position, velocity, forward, up);
    }

    bool set_listener_position(const vec3 &position)
    {
        return detail::get_sound_manager_instance().set_listener_position(position);
    }

    bool set_listener_velocity(const vec3 &velocity)
    {
        return detail::get_sound_manager_instance().set_listener_velocity(velocity);
    }

    bool set_listener_forward(const vec3 &forward)
    {
        return detail::get_sound_manager_instance().set_listener_forward(forward);
    }

    bool set_listener_up(const vec3 &up)
    {
        return detail::get_sound_manager_instance().set_listener_up(up);
    }

    vec3 get_listener_position()
    {
        return detail::get_sound_manager_instance().get_listener_position();
    }

    vec3 get_listener_velocity()
    {
        return detail::get_sound_manager_instance().get_listener_velocity();
    }

    vec3 get_listener_forward()
    {
        return detail::get_sound_manager_instance().get_listener_forward();
    }

    vec3 get_listener_up()
    {
        return detail::get_sound_manager_instance().get_listener_up();
    }

    const std::string get_error()
    {
        return detail::get_sound_manager_instance().get_error();
    }

    void clear_error()
    {
        detail::get_sound_manager_instance().clear_error();
    }

    bool is_handle_valid(size_t handle)
    {
        return detail::get_sound_manager_instance().is_handle_valid(handle);
    }
} // namespace soundcoe
