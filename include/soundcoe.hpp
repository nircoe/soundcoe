#pragma once

#include <soundcoe/core/error.hpp>
#include <soundcoe/core/types.hpp>
#include <soundcoe/utils/math.hpp>
#include <string>
#include <expected>
#include <cstddef>

namespace soundcoe
{
    namespace internal
    {
        class sound_manager;

        sound_manager& get_sound_manager_instance();
    }
}

namespace soundcoe
{
    // Settings for initialize(), bundled in one struct.
    struct init_config
    {
        std::string audio_root_directory = "assets/audio";
        std::string sound_subdir = "sfx";
        std::string music_subdir = "music";
        std::size_t max_sources = 64;
        std::size_t max_cache_size_mb = UNLIMITED_CACHE;
        LogLevel level = LogLevel::DEBUG;
    };

    /**
     * Starts soundcoe and loads the optional "general" folder under audio_root_directory, for audio that stays loaded
     * until shutdown(). Call this before anything else, and call shutdown() before initializing again. A relative
     * audio_root_directory resolves from the working directory. sound_subdir and music_subdir are the folder names for
     * sound effects and music inside "general" and each scene folder. max_sources is how many sound effects and music
     * tracks can play at once. max_cache_size_mb is the size limit in MB for the cache of loaded audio, UNLIMITED_CACHE
     * removes the limit.
     * Errors: already_initialized, invalid_argument (empty directory), directory_not_found, filesystem_error,
     * OpenAL error.
     */
    [[nodiscard]] std::expected<void, error> initialize(
        const std::string &audio_root_directory, std::size_t max_sources = 64,
        std::size_t max_cache_size_mb = UNLIMITED_CACHE, const std::string &sound_subdir = "sfx",
        const std::string &music_subdir = "music", LogLevel level = LogLevel::DEBUG);

    /**
     * Starts soundcoe with the settings from an init_config and loads the optional "general" folder under its
     * audio_root_directory, for audio that stays loaded until shutdown(). Call this before anything else, and call
     * shutdown() before initializing again.
     * Errors: already_initialized, invalid_argument (empty directory), directory_not_found, filesystem_error,
     * OpenAL error.
     */
    [[nodiscard]] std::expected<void, error> initialize(const init_config &config);

    /**
     * Stops all audio, unloads all loaded audio files and closes the audio device. Call initialize() to use soundcoe
     * again.
     */
    void shutdown();

    // Checks if soundcoe is initialized.
    [[nodiscard]] bool is_initialized();

    /**
     * Loads a scene's audio into memory, so its files can be played. scene_name is the scene's folder under the audio
     * root. Every audio file in it is loaded, subfolders included. A file that fails to load is logged and skipped. It
     * doesn't fail the call.
     * Errors: not_initialized, invalid_argument (empty name), directory_not_found, filesystem_error.
     */
    [[nodiscard]] std::expected<void, error> preload_scene(const std::string &scene_name);

    // Unloads a scene's audio from memory. Errors: not_initialized.
    [[nodiscard]] std::expected<void, error> unload_scene(const std::string &scene_name);

    // Checks if a scene is loaded.
    [[nodiscard]] bool is_scene_loaded(const std::string &scene_name);

    /**
     * Call once per frame. Fades only advance when it runs. It also removes the handles of sound effects and music
     * tracks that were cut off, like when a higher priority one took their place.
     */
    void update();

    /**
     * Plays a sound effect from the sound_subdir folder of "general" or a loaded scene. When max_sources (set in
     * initialize()) are all in use, the lowest priority sound effect or music track is stopped to make room. If all of
     * them have a higher priority, resource_exhausted is returned.
     * Errors: not_initialized, file_not_found, unsupported_format, audio_decode_failure, filesystem_error,
     * resource_exhausted, OpenAL error.
     */
    [[nodiscard]] std::expected<sound_handle, error> play_sound(
        const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
        sound_priority priority = sound_priority::medium);

    /**
     * Plays a sound effect at a position in the world, from the sound_subdir folder of "general" or a loaded scene. The
     * velocity is used for the doppler effect. When max_sources (set in initialize()) are all in use, the lowest
     * priority sound effect or music track is stopped to make room. If all of them have a higher priority,
     * resource_exhausted is returned.
     * Errors: not_initialized, file_not_found, unsupported_format, audio_decode_failure, filesystem_error,
     * resource_exhausted, OpenAL error.
     */
    [[nodiscard]] std::expected<sound_handle, error> play_sound3d(
        const std::string &filename, const vec3 &position, const vec3 &velocity = vec3::zero(),
        float volume = 1.0f, float pitch = 1.0f, bool loop = false,
        sound_priority priority = sound_priority::medium);

    /**
     * Plays a music track from the music_subdir folder of "general" or a loaded scene. Loops by default. When
     * max_sources (set in initialize()) are all in use, the lowest priority sound effect or music track is stopped to
     * make room. If all of them have a higher priority, resource_exhausted is returned.
     * Errors: not_initialized, file_not_found, unsupported_format, audio_decode_failure, filesystem_error,
     * resource_exhausted, OpenAL error.
     */
    [[nodiscard]] std::expected<music_handle, error> play_music(
        const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
        sound_priority priority = sound_priority::critical);

    // Pauses a sound effect. Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
    [[nodiscard]] std::expected<void, error> pause_sound(sound_handle handle);

    // Pauses a music track. Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
    [[nodiscard]] std::expected<void, error> pause_music(music_handle handle);

    // Pauses all sound effects.
    void pause_all_sounds();

    // Pauses all music tracks.
    void pause_all_music();

    // Pauses all sound effects and music tracks.
    void pause_all();

    /**
     * Resumes a paused sound effect.
     * Errors: invalid_handle, source_inactive (playback was cut off), invalid_state (not paused), OpenAL error.
     */
    [[nodiscard]] std::expected<void, error> resume_sound(sound_handle handle);

    /**
     * Resumes a paused music track.
     * Errors: invalid_handle, source_inactive (playback was cut off), invalid_state (not paused), OpenAL error.
     */
    [[nodiscard]] std::expected<void, error> resume_music(music_handle handle);

    // Resumes all paused sound effects.
    void resume_all_sounds();

    // Resumes all paused music tracks.
    void resume_all_music();

    // Resumes all paused sound effects and music tracks.
    void resume_all();

    /**
     * Stops a sound effect. The handle can't be used afterwards.
     * Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
     */
    [[nodiscard]] std::expected<void, error> stop_sound(sound_handle handle);

    /**
     * Stops a music track. The handle can't be used afterwards.
     * Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
     */
    [[nodiscard]] std::expected<void, error> stop_music(music_handle handle);

    // Stops all sound effects.
    void stop_all_sounds();

    // Stops all music tracks.
    void stop_all_music();

    // Stops all sound effects and music tracks.
    void stop_all();

    // Sets a sound effect's volume. Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
    [[nodiscard]] std::expected<void, error> set_sound_volume(sound_handle handle, float volume);

    // Sets a music track's volume. Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
    [[nodiscard]] std::expected<void, error> set_music_volume(music_handle handle, float volume);

    // Sets a sound effect's pitch. Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
    [[nodiscard]] std::expected<void, error> set_sound_pitch(sound_handle handle, float pitch);

    // Sets a music track's pitch. Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
    [[nodiscard]] std::expected<void, error> set_music_pitch(music_handle handle, float pitch);

    /**
     * Sets a sound effect's position in the world.
     * Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
     */
    [[nodiscard]] std::expected<void, error> set_sound_position(sound_handle handle, const vec3 &position);

    /**
     * Sets a sound effect's velocity, used for the doppler effect.
     * Errors: invalid_handle, source_inactive (playback was cut off), OpenAL error.
     */
    [[nodiscard]] std::expected<void, error> set_sound_velocity(sound_handle handle, const vec3 &velocity);

    // Checks if a sound effect is playing. Errors: invalid_handle, source_inactive (playback was cut off).
    [[nodiscard]] std::expected<bool, error> is_sound_playing(sound_handle handle);

    // Checks if a music track is playing. Errors: invalid_handle, source_inactive (playback was cut off).
    [[nodiscard]] std::expected<bool, error> is_music_playing(music_handle handle);

    // Checks if a sound effect is paused. Errors: invalid_handle, source_inactive (playback was cut off).
    [[nodiscard]] std::expected<bool, error> is_sound_paused(sound_handle handle);

    // Checks if a music track is paused. Errors: invalid_handle, source_inactive (playback was cut off).
    [[nodiscard]] std::expected<bool, error> is_music_paused(music_handle handle);

    // Checks if a sound effect is stopped. Errors: invalid_handle, source_inactive (playback was cut off).
    [[nodiscard]] std::expected<bool, error> is_sound_stopped(sound_handle handle);

    // Checks if a music track is stopped. Errors: invalid_handle, source_inactive (playback was cut off).
    [[nodiscard]] std::expected<bool, error> is_music_stopped(music_handle handle);

    // Gets the number of sound effects that were started and not stopped yet, finished ones included.
    [[nodiscard]] std::size_t get_active_sounds_count();

    // Gets the number of music tracks that were started and not stopped yet, finished ones included.
    [[nodiscard]] std::size_t get_active_music_count();

    /**
     * Plays a sound effect from the sound_subdir folder of "general" or a loaded scene and fades it in from silence to
     * volume over duration seconds. The fade advances in update(). When max_sources (set in initialize()) are all in
     * use, the lowest priority sound effect or music track is stopped to make room. If all of them have a higher
     * priority, resource_exhausted is returned.
     * Errors: invalid_argument (duration <= 0), not_initialized, file_not_found, unsupported_format,
     * audio_decode_failure, filesystem_error, resource_exhausted, OpenAL error.
     */
    [[nodiscard]] std::expected<sound_handle, error> fade_in_sound(
        const std::string &filename, float duration, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
        sound_priority priority = sound_priority::medium);

    /**
     * Plays a music track from the music_subdir folder of "general" or a loaded scene and fades it in from silence to
     * volume over duration seconds. Loops by default. The fade advances in update(). When max_sources (set in
     * initialize()) are all in use, the lowest priority sound effect or music track is stopped to make room. If all of
     * them have a higher priority, resource_exhausted is returned.
     * Errors: invalid_argument (duration <= 0), not_initialized, file_not_found, unsupported_format,
     * audio_decode_failure, filesystem_error, resource_exhausted, OpenAL error.
     */
    [[nodiscard]] std::expected<music_handle, error> fade_in_music(
        const std::string &filename, float duration, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
        sound_priority priority = sound_priority::critical);

    /**
     * Fades a sound effect to silence over duration seconds, then stops it. The fade advances in update(), and the
     * handle can't be used once it ends.
     * Errors: invalid_handle, source_inactive (playback was cut off), invalid_argument (duration <= 0),
     * invalid_state (paused or stopped).
     */
    [[nodiscard]] std::expected<void, error> fade_out_sound(sound_handle handle, float duration);

    /**
     * Fades a music track to silence over duration seconds, then stops it. The fade advances in update(), and the
     * handle can't be used once it ends.
     * Errors: invalid_handle, source_inactive (playback was cut off), invalid_argument (duration <= 0),
     * invalid_state (paused or stopped).
     */
    [[nodiscard]] std::expected<void, error> fade_out_music(music_handle handle, float duration);

    /**
     * Fades a sound effect from its current volume to target_volume over duration seconds. The fade advances in
     * update(). A target_volume of 0 stops it, and the handle can't be used afterwards.
     * Errors: invalid_handle, source_inactive (playback was cut off), invalid_argument (duration <= 0 or
     * target_volume < 0), invalid_state (paused or stopped).
     */
    [[nodiscard]] std::expected<void, error> fade_to_volume_sound(sound_handle handle, float target_volume,
                                                                  float duration);

    /**
     * Fades a music track from its current volume to target_volume over duration seconds. The fade advances in
     * update(). A target_volume of 0 stops it, and the handle can't be used afterwards.
     * Errors: invalid_handle, source_inactive (playback was cut off), invalid_argument (duration <= 0 or
     * target_volume < 0), invalid_state (paused or stopped).
     */
    [[nodiscard]] std::expected<void, error> fade_to_volume_music(music_handle handle, float target_volume,
                                                                  float duration);

    // Sets the master volume multiplier for all audio.
    void set_master_volume(float volume);

    // Sets the master volume multiplier for sound effects.
    void set_master_sounds_volume(float volume);

    // Sets the master volume multiplier for music tracks.
    void set_master_music_volume(float volume);

    // Sets the master pitch multiplier for all audio.
    void set_master_pitch(float pitch);

    // Sets the master pitch multiplier for sound effects.
    void set_master_sounds_pitch(float pitch);

    // Sets the master pitch multiplier for music tracks.
    void set_master_music_pitch(float pitch);

    // Gets the master volume multiplier for all audio.
    [[nodiscard]] float get_master_volume();

    // Gets the master volume multiplier for sound effects.
    [[nodiscard]] float get_master_sounds_volume();

    // Gets the master volume multiplier for music tracks.
    [[nodiscard]] float get_master_music_volume();

    // Gets the master pitch multiplier for all audio.
    [[nodiscard]] float get_master_pitch();

    // Gets the master pitch multiplier for sound effects.
    [[nodiscard]] float get_master_sounds_pitch();

    // Gets the master pitch multiplier for music tracks.
    [[nodiscard]] float get_master_music_pitch();

    // Mutes all sound effects. Unmuting restores their volume.
    void mute_all_sounds();

    // Mutes all music tracks. Unmuting restores their volume.
    void mute_all_music();

    // Mutes all sound effects and music tracks. Unmuting restores their volume.
    void mute_all();

    // Unmutes all sound effects. Doesn't undo mute_all(), use unmute_all() for that.
    void unmute_all_sounds();

    // Unmutes all music tracks. Doesn't undo mute_all(), use unmute_all() for that.
    void unmute_all_music();

    // Unmutes all sound effects and music tracks.
    void unmute_all();

    // Checks if all audio is muted with mute_all().
    [[nodiscard]] bool is_muted();

    // Checks if sound effects are muted with mute_all_sounds().
    [[nodiscard]] bool is_sounds_muted();

    // Checks if music tracks are muted with mute_all_music().
    [[nodiscard]] bool is_music_muted();

    /**
     * Sets the listener's position, velocity, forward and up at once. forward and up must be normalized.
     * Errors: OpenAL error.
     */
    [[nodiscard]] std::expected<void, error> update_listener(
        const vec3 &position, const vec3 &velocity, const vec3 &forward, const vec3 &up = vec3::up());

    // Sets the listener's position. Errors: OpenAL error.
    [[nodiscard]] std::expected<void, error> set_listener_position(const vec3 &position);

    // Sets the listener's velocity, used for the doppler effect. Errors: OpenAL error.
    [[nodiscard]] std::expected<void, error> set_listener_velocity(const vec3 &velocity);

    // Sets the listener's forward direction. forward must be normalized. Errors: OpenAL error.
    [[nodiscard]] std::expected<void, error> set_listener_forward(const vec3 &forward);

    // Sets the listener's up direction. up must be normalized. Errors: OpenAL error.
    [[nodiscard]] std::expected<void, error> set_listener_up(const vec3 &up);

    // Gets the listener's position.
    [[nodiscard]] vec3 get_listener_position();

    // Gets the listener's velocity.
    [[nodiscard]] vec3 get_listener_velocity();

    // Gets the listener's forward direction.
    [[nodiscard]] vec3 get_listener_forward();

    // Gets the listener's up direction.
    [[nodiscard]] vec3 get_listener_up();

    /**
     * Checks that a handle isn't INVALID_SOUND_HANDLE or INVALID_MUSIC_HANDLE. It doesn't check that the audio is still
     * playing.
     */
    [[nodiscard]] bool is_handle_valid(std::size_t handle);
} // namespace soundcoe
