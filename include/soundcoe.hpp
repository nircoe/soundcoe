#pragma once

#include <soundcoe/core/types.hpp>
#include <soundcoe/utils/math.hpp>
#include <string>
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
    /**
     * @brief Configuration bundle for initialize(), for callers that want to hold/forward init
     *        settings as a single value (e.g. gamecoe's game::create()) instead of six positional
     *        arguments.
     */
    struct init_config
    {
        std::string audio_root_directory = "assets/audio";
        std::string sound_subdir = "sfx";
        std::string music_subdir = "music";
        size_t max_sources = 64;
        size_t max_cache_size_mb = UNLIMITED_CACHE;
        LogLevel level = LogLevel::DEBUG;
    };

    /**
     * @brief Initializes soundcoe with the specified configuration.
     *
     * Sets up an audio context, resource management, and loads the "general" audio directory if it exists.
     * This function must be called before using any other soundcoe functionality.
     *
     * @param audio_root_directory Path to the root audio directory (relative to executable). The
     *                          "general" subdirectory is optional and will be automatically loaded
     *                          during initialization if exists, and will be loaded till soundcoe::shutdown() will be called.
     * @param max_sources Maximum number of concurrent audio sources that can be played simultaneously.
     *                   Higher values allow more concurrent audio but consume more system resources.
     *                   Default is 64.
     * @param max_cache_size_mb Maximum size in megabytes for the audio buffer cache. Larger values keep
     *                       more audio files in memory for faster playback but consume more RAM.
     *                       Default is soundcoe::UNLIMITED_CACHE, so nothing evicts during development;
     *                       set a real budget once you've profiled actual usage for shipping.
     * @param sound_subdir Name of the subdirectory within each audio directory (general, scenes) that
     *                    contains sound effects. Default is "sfx".
     * @param music_subdir Name of the subdirectory within each audio directory (general, scenes) that
     *                    contains music files. Default is "music".
     * @param level Logging level for soundcoe operations. Controls the verbosity of log output.
     *              Default is LogLevel::DEBUG.
     *
     * @return true if initialization was successful, false if it failed (e.g., invalid directory,
     *         OpenAL initialization failure, or system already initialized).
     *
     * @note This function should only be called once at application startup. Multiple calls will
     *       return false. Call shutdown() before calling initialize() again if needed.
     *
     * @example
     * // Basic initialization - will load ./audio/general/sfx/ and ./audio/general/music/ if they exist
     * if (!soundcoe::initialize("./audio")) {
     *     std::cerr << "Failed to initialize audio: " << soundcoe::get_error() << std::endl;
     * }
     *
     * @example
     * // Custom subdirectory names - will load ./game_audio/general/sounds/ and ./game_audio/general/bgm/
     * if (!soundcoe::initialize("./game_audio", 64, 128, "sounds", "bgm", LogLevel::WARNING)) {
     *     // Handle initialization failure
     * }
     */
    bool initialize(const std::string &audio_root_directory, size_t max_sources = 64,
                           size_t max_cache_size_mb = UNLIMITED_CACHE, const std::string &sound_subdir = "sfx",
                           const std::string &music_subdir = "music", LogLevel level = LogLevel::DEBUG);

    /**
     * @brief Initializes soundcoe from an init_config bundle. Forwards to the flat-parameter
     *        overload above - see its documentation for behavior/return value.
     */
    bool initialize(const init_config &config);

    /**
     * @brief Shuts down soundcoe and releases all resources.
     * 
     * Stops all playing audio, unloads all cached audio files, and cleans up the audio context.
     * After calling this function, initialize() must be called again before using soundcoe.
     * 
     * @note This function should be called at application shutdown or before re-initializing.
     */
    void shutdown();

    /**
     * @brief Checks if soundcoe is currently initialized.
     * 
     * @return true if soundcoe has been successfully initialized, false otherwise.
     */
    bool is_initialized();

    /**
     * @brief Preloads all audio files from a scene directory.
     * 
     * Loads audio files from audio_root_directory/{scene_name}/{sound_subdir}/ and 
     * audio_root_directory/{scene_name}/{music_subdir}/ into memory for faster playback.
     * 
     * @param scene_name Name of the scene directory to preload.
     * @return true if scene was loaded successfully, false if directory doesn't exist or loading failed.
     */
    bool preload_scene(const std::string &scene_name);

    /**
     * @brief Unloads a previously loaded scene and frees its audio resources.
     * 
     * @param scene_name Name of the scene directory to unload.
     * @return true if scene was unloaded successfully, false if scene wasn't loaded.
     */
    bool unload_scene(const std::string &scene_name);

    /**
     * @brief Checks if a scene is currently loaded.
     * 
     * @param scene_name Name of the scene to check.
     * @return true if the scene is loaded, false otherwise.
     */
    bool is_scene_loaded(const std::string &scene_name);

    // in the future: bool preload_scene/unload_scene/is_scene_loaded(const Scene &scene); with gamecoe::Scene object!

    /**
     * @brief Updates soundcoe internal systems (fade effects, cleanup).
     * 
     * Should be called regularly (once per frame) to process fade effects
     * and clean up stopped audio sources.
     */
    void update();

    /**
     * @brief Plays a sound file with specified properties.
     *
     * @param filename Name of the sound file to play (should be in a loaded <general or scene>/{sound_subdir}/ subdirectory).
     * @param volume Volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the sound. Default is false.
     * @param priority Sound priority for resource allocation. Default is Medium.
     * @return sound_handle to control the playing sound, or INVALID_SOUND_HANDLE (equal to 0) if playback failed.
     */
    sound_handle play_sound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                 sound_priority priority = sound_priority::medium);

    /**
     * @brief Plays a 3D positioned sound with spatial audio properties.
     *
     * @param filename Name of the sound file to play (should be in a loaded <general or scene>/{sound_subdir}/ subdirectory).
     * @param position 3D world position of the sound source.
     * @param velocity 3D velocity vector for doppler effect. Default is zero.
     * @param volume Volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the sound. Default is false.
     * @param priority Sound priority for resource allocation. Default is Medium.
     * @return sound_handle to control the playing sound, or INVALID_SOUND_HANDLE (equal to 0) if playback failed.
     */
    sound_handle play_sound3d(const std::string &filename, const vec3 &position, const vec3 &velocity = vec3::zero(),
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   sound_priority priority = sound_priority::medium);

    /**
     * @brief Plays a music file with specified properties.
     *
     * @param filename Name of the music file to play (should be in a loaded <general or scene>/{music_subdir}/ subdirectory).
     * @param volume Volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the music. Default is true.
     * @param priority Music priority for resource allocation. Default is Critical.
     * @return music_handle to control the playing music, or INVALID_MUSIC_HANDLE (equal to 0) if playback failed.
     */
    music_handle play_music(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                 sound_priority priority = sound_priority::critical);

    /**
     * @brief Pauses a specific sound.
     * 
     * @param handle Handle of the sound to pause.
     * @return true if successfully paused, false if handle is invalid or sound not playing.
     */
    bool pause_sound(sound_handle handle);

    /**
     * @brief Pauses a specific music track.
     * 
     * @param handle Handle of the music to pause.
     * @return true if successfully paused, false if handle is invalid or music not playing.
     */
    bool pause_music(music_handle handle);

    /**
     * @brief Pauses all currently playing sounds.
     */
    void pause_all_sounds();

    /**
     * @brief Pauses all currently playing music tracks.
     */
    void pause_all_music();

    /**
     * @brief Pauses all currently playing sounds and music.
     */
    void pause_all();

    /**
     * @brief Resumes a paused sound.
     * 
     * @param handle Handle of the sound to resume.
     * @return true if successfully resumed, false if handle is invalid or sound not paused.
     */
    bool resume_sound(sound_handle handle);

    /**
     * @brief Resumes a paused music track.
     * 
     * @param handle Handle of the music to resume.
     * @return true if successfully resumed, false if handle is invalid or music not paused.
     */
    bool resume_music(music_handle handle);

    /**
     * @brief Resumes all paused sounds.
     */
    void resume_all_sounds();

    /**
     * @brief Resumes all paused music tracks.
     */
    void resume_all_music();

    /**
     * @brief Resumes all paused sounds and music.
     */
    void resume_all();

    /**
     * @brief Stops a playing or paused sound.
     * 
     * @param handle Handle of the sound to stop.
     * @return true if successfully stopped, false if handle is invalid.
     */
    bool stop_sound(sound_handle handle);

    /**
     * @brief Stops a playing or paused music track.
     * 
     * @param handle Handle of the music to stop.
     * @return true if successfully stopped, false if handle is invalid.
     */
    bool stop_music(music_handle handle);

    /**
     * @brief Stops all currently active sounds.
     */
    void stop_all_sounds();

    /**
     * @brief Stops all currently active music tracks.
     */
    void stop_all_music();

    /**
     * @brief Stops all currently active sounds and music.
     */
    void stop_all();

    /**
     * @brief Sets the volume of a specific sound.
     * 
     * @param handle Handle of the sound to modify.
     * @param volume New volume level.
     * @return true if successfully set, false if handle is invalid.
     */
    bool set_sound_volume(sound_handle handle, float volume);

    /**
     * @brief Sets the volume of a specific music track.
     * 
     * @param handle Handle of the music to modify.
     * @param volume New volume level.
     * @return true if successfully set, false if handle is invalid.
     */
    bool set_music_volume(music_handle handle, float volume);

    /**
     * @brief Sets the pitch of a specific sound.
     * 
     * @param handle Handle of the sound to modify.
     * @param pitch New pitch multiplier.
     * @return true if successfully set, false if handle is invalid.
     */
    bool set_sound_pitch(sound_handle handle, float pitch);

    /**
     * @brief Sets the pitch of a specific music track.
     * 
     * @param handle Handle of the music to modify.
     * @param pitch New pitch multiplier.
     * @return true if successfully set, false if handle is invalid.
     */
    bool set_music_pitch(music_handle handle, float pitch);

    /**
     * @brief Sets the 3D position of a sound source.
     * 
     * @param handle Handle of the sound to modify.
     * @param position New 3D world position.
     * @return true if successfully set, false if handle is invalid.
     */
    bool set_sound_position(sound_handle handle, const vec3 &position);

    /**
     * @brief Sets the 3D velocity of a sound source for doppler effect.
     * 
     * @param handle Handle of the sound to modify.
     * @param velocity New 3D velocity vector.
     * @return true if successfully set, false if handle is invalid.
     */
    bool set_sound_velocity(sound_handle handle, const vec3 &velocity);

    /**
     * @brief Checks if a sound is currently playing.
     * 
     * @param handle Handle of the sound to check.
     * @return true if sound is playing, false if paused, stopped, or handle is invalid.
     */
    bool is_sound_playing(sound_handle handle);

    /**
     * @brief Checks if a music track is currently playing.
     * 
     * @param handle Handle of the music to check.
     * @return true if music is playing, false if paused, stopped, or handle is invalid.
     */
    bool is_music_playing(music_handle handle);

    /**
     * @brief Checks if a sound is currently paused.
     * 
     * @param handle Handle of the sound to check.
     * @return true if sound is paused, false if playing, stopped, or handle is invalid.
     */
    bool is_sound_paused(sound_handle handle);

    /**
     * @brief Checks if a music track is currently paused.
     * 
     * @param handle Handle of the music to check.
     * @return true if music is paused, false if playing, stopped, or handle is invalid.
     */
    bool is_music_paused(music_handle handle);

    /**
     * @brief Checks if a sound is currently stopped.
     * 
     * @param handle Handle of the sound to check.
     * @return true if sound is stopped, false if playing, paused, or handle is invalid.
     */
    bool is_sound_stopped(sound_handle handle);

    /**
     * @brief Checks if a music track is currently stopped.
     * 
     * @param handle Handle of the music to check.
     * @return true if music is stopped, false if playing, paused, or handle is invalid.
     */
    bool is_music_stopped(music_handle handle);

    /**
     * @brief Gets the number of currently active sound sources.
     * 
     * @return Number of sounds that are active.
     */
    size_t get_active_sounds_count();

    /**
     * @brief Gets the number of currently active music tracks.
     * 
     * @return Number of music tracks that are active.
     */
    size_t get_active_music_count();

    /**
     * @brief Plays a sound with a fade-in effect from silence to target volume.
     *
     * @param filename Name of the sound file to play (relative to loaded sound effects directories).
     * @param duration Fade-in duration in seconds.
     * @param volume Target volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the sound. Default is false.
     * @param priority Sound priority for resource allocation. Default is Medium.
     * @return sound_handle to control the playing sound, or INVALID_SOUND_HANDLE (equal to 0) if playback failed.
     */
    sound_handle fade_in_sound(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   sound_priority priority = sound_priority::medium);

    /**
     * @brief Plays music with a fade-in effect from silence to target volume.
     *
     * @param filename Name of the music file to play (relative to loaded music directories).
     * @param duration Fade-in duration in seconds.
     * @param volume Target volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the music. Default is true.
     * @param priority Music priority for resource allocation. Default is Critical.
     * @return music_handle to control the playing music, or INVALID_MUSIC_HANDLE (equal to 0) if playback failed.
     */
    music_handle fade_in_music(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                   sound_priority priority = sound_priority::critical);

    /**
     * @brief Fades out a sound from current volume to silence, then stops it.
     * 
     * @param handle Handle of the sound to fade out.
     * @param duration Fade-out duration in seconds.
     * @return true if fade started successfully, false if handle is invalid.
     */
    bool fade_out_sound(sound_handle handle, float duration);

    /**
     * @brief Fades out music from current volume to silence, then stops it.
     * 
     * @param handle Handle of the music to fade out.
     * @param duration Fade-out duration in seconds.
     * @return true if fade started successfully, false if handle is invalid.
     */
    bool fade_out_music(music_handle handle, float duration);

    /**
     * @brief Fades a sound from current volume to a target volume over time.
     * 
     * @param handle Handle of the sound to fade.
     * @param target_volume Target volume level.
     * @param duration Fade duration in seconds.
     * @return true if fade started successfully, false if handle is invalid.
     */
    bool fade_to_volume_sound(sound_handle handle, float target_volume, float duration);

    /**
     * @brief Fades music from current volume to a target volume over time.
     * 
     * @param handle Handle of the music to fade.
     * @param target_volume Target volume level.
     * @param duration Fade duration in seconds.
     * @return true if fade started successfully, false if handle is invalid.
     */
    bool fade_to_volume_music(music_handle handle, float target_volume, float duration);

    /**
     * @brief Sets the master volume multiplier for all audio (sounds and music).
     * 
     * @param volume Master volume multiplier level.
     * @return true if successfully set, false on error.
     */
    bool set_master_volume(float volume);

    /**
     * @brief Sets the master volume multiplier for all sound effects.
     * 
     * @param volume Sounds master volume level multiplier.
     * @return true if successfully set, false on error.
     */
    bool set_master_sounds_volume(float volume);

    /**
     * @brief Sets the master volume multiplier for all music tracks.
     * 
     * @param volume Music master volume level multiplier.
     * @return true if successfully set, false on error.
     */
    bool set_master_music_volume(float volume);

    /**
     * @brief Sets the master pitch multiplier for all audio (sounds and music).
     * 
     * @param pitch Master pitch multiplier.
     * @return true if successfully set, false on error.
     */
    bool set_master_pitch(float pitch);

    /**
     * @brief Sets the master pitch multiplier for all sounds effects.
     * 
     * @param pitch Sounds master pitch multiplier.
     * @return true if successfully set, false on error.
     */
    bool set_master_sounds_pitch(float pitch);

    /**
     * @brief Sets the master pitch multiplier for all music tracks.
     * 
     * @param pitch Music master pitch multiplier.
     * @return true if successfully set, false on error.
     */
    bool set_master_music_pitch(float pitch);

    /**
     * @brief Gets the current master volume multiplier for all audio.
     *
     * @return Current master volume level multiplier.
     */
    float get_master_volume();

    /**
     * @brief Gets the current master volume multiplier for sound effects.
     *
     * @return Current sounds master volume level multiplier.
     */
    float get_master_sounds_volume();

    /**
     * @brief Gets the current master volume multiplier for music tracks.
     *
     * @return Current music master volume level multiplier.
     */
    float get_master_music_volume();

    /**
     * @brief Gets the current master pitch multiplier for all audio.
     *
     * @return Current master pitch multiplier.
     */
    float get_master_pitch();

    /**
     * @brief Gets the current master pitch multiplier for sound effects.
     *
     * @return Current sounds master pitch multiplier.
     */
    float get_master_sounds_pitch();

    /**
     * @brief Gets the current master pitch multiplier for music tracks.
     *
     * @return Current music master pitch multiplier.
     */
    float get_master_music_pitch();

    /**
     * @brief Mutes all sound effects while preserving their volume settings.
     */
    void mute_all_sounds();

    /**
     * @brief Mutes all music tracks while preserving their volume settings.
     */
    void mute_all_music();

    /**
     * @brief Mutes all audio (sounds and music) while preserving volume settings.
     */
    void mute_all();

    /**
     * @brief Unmutes all sound effects, restoring their previous volume settings.
     */
    void unmute_all_sounds();

    /**
     * @brief Unmutes all music, restoring their previous volume settings.
     */
    void unmute_all_music();

    /**
     * @brief Unmutes all audio (sounds and music), restoring previous volume settings.
     */
    void unmute_all();

    /**
     * @brief Checks if all audio is currently muted.
     * 
     * @return true if all audio is muted, false otherwise.
     */
    bool is_muted();

    /**
     * @brief Checks if all sound effects are currently muted.
     * 
     * @return true if sounds are muted, false otherwise.
     */
    bool is_sounds_muted();

    /**
     * @brief Checks if all music tracks are currently muted.
     * 
     * @return true if music is muted, false otherwise.
     */
    bool is_music_muted();

    /**
     * @brief Updates all 3D audio listener properties at once.
     * 
     * @param position 3D world position of the listener.
     * @param velocity 3D velocity vector for doppler effect.
     * @param forward Forward direction vector (must be normalized).
     * @param up Up direction vector (must be normalized). Default is vec3::up().
     * @return true if successfully updated, false on error.
     */
    bool update_listener(const vec3 &position, const vec3 &velocity, const vec3 &forward, const vec3 &up = vec3::up());

    /**
     * @brief Sets the 3D position of the audio listener.
     * 
     * @param position 3D world position of the listener.
     * @return true if successfully set, false on error.
     */
    bool set_listener_position(const vec3 &position);

    /**
     * @brief Sets the velocity of the audio listener for doppler effect.
     * 
     * @param velocity 3D velocity vector of the listener.
     * @return true if successfully set, false on error.
     */
    bool set_listener_velocity(const vec3 &velocity);

    /**
     * @brief Sets the forward direction of the audio listener.
     * 
     * @param forward Forward direction vector (must be normalized).
     * @return true if successfully set, false on error.
     */
    bool set_listener_forward(const vec3 &forward);

    /**
     * @brief Sets the up direction of the audio listener.
     * 
     * @param up Up direction vector (must be normalized).
     * @return true if successfully set, false on error.
     */
    bool set_listener_up(const vec3 &up);

    /**
     * @brief Gets the current 3D position of the audio listener.
     * 
     * @return Current listener position.
     */
    vec3 get_listener_position();

    /**
     * @brief Gets the current velocity of the audio listener.
     * 
     * @return Current listener velocity vector.
     */
    vec3 get_listener_velocity();

    /**
     * @brief Gets the current forward direction of the audio listener.
     * 
     * @return Current listener forward direction vector.
     */
    vec3 get_listener_forward();

    /**
     * @brief Gets the current up direction of the audio listener.
     * 
     * @return Current listener up direction vector.
     */
    vec3 get_listener_up();

    /**
     * @brief Gets the last error message and clears the error state.
     * 
     * @return Last error message, or empty string if no error occurred.
     */
    const std::string get_error();

    /**
     * @brief Clears the current error state without returning the message.
     */
    void clear_error();

    /**
     * @brief Checks if a handle (sound or music) is valid and active.
     * 
     * @param handle Handle to validate.
     * @return true if handle is valid and represents active audio, false otherwise.
     */
    bool is_handle_valid(size_t handle);
} // namespace soundcoe
