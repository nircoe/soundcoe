#pragma once

#include <soundcoe/utils/math.hpp>
#include <string>

namespace soundcoe
{
    namespace detail
    {
        class SoundManager;

        SoundManager& getSoundManagerInstance();
    }
}

namespace soundcoe
{
    /**
     * @brief Initializes soundcoe with the specified configuration.
     *
     * Sets up an audio context, resource management, and loads the "general" audio directory if it exists.
     * This function must be called before using any other soundcoe functionality.
     *
     * @param audioRootDirectory Path to the root audio directory (relative to executable). The
     *                          "general" subdirectory is optional and will be automatically loaded
     *                          during initialization if exists, and will be loaded till soundcoe::shutdown() will be called.
     * @param maxSources Maximum number of concurrent audio sources that can be played simultaneously.
     *                   Higher values allow more concurrent audio but consume more system resources.
     *                   Default is 32.
     * @param maxCacheSizeMB Maximum size in megabytes for the audio buffer cache. Larger values keep
     *                       more audio files in memory for faster playback but consume more RAM.
     *                       Use soundcoe::UNLIMITED_CACHE during development to measure actual usage.
     *                       Default is 64 MB.
     * @param soundSubdir Name of the subdirectory within each audio directory (general, scenes) that
     *                    contains sound effects. Default is "sfx".
     * @param musicSubdir Name of the subdirectory within each audio directory (general, scenes) that
     *                    contains music files. Default is "music".
     * @param level Logging level for soundcoe operations. Controls the verbosity of log output.
     *              Default is LogLevel::INFO.
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
     *     std::cerr << "Failed to initialize audio: " << soundcoe::getError() << std::endl;
     * }
     *
     * @example
     * // Custom subdirectory names - will load ./game_audio/general/sounds/ and ./game_audio/general/bgm/
     * if (!soundcoe::initialize("./game_audio", 64, 128, "sounds", "bgm", LogLevel::WARNING)) {
     *     // Handle initialization failure
     * }
     */
    bool initialize(const std::string &audioRootDirectory, size_t maxSources = 32,
                           size_t maxCacheSizeMB = 64, const std::string &soundSubdir = "sfx",
                           const std::string &musicSubdir = "music", LogLevel level = LogLevel::INFO);

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
    bool isInitialized();

    /**
     * @brief Preloads all audio files from a scene directory.
     * 
     * Loads audio files from audioRootDirectory/{sceneName}/{soundSubdir}/ and 
     * audioRootDirectory/{sceneName}/{musicSubdir}/ into memory for faster playback.
     * 
     * @param sceneName Name of the scene directory to preload.
     * @return true if scene was loaded successfully, false if directory doesn't exist or loading failed.
     */
    bool preloadScene(const std::string &sceneName);

    /**
     * @brief Unloads a previously loaded scene and frees its audio resources.
     * 
     * @param sceneName Name of the scene directory to unload.
     * @return true if scene was unloaded successfully, false if scene wasn't loaded.
     */
    bool unloadScene(const std::string &sceneName);

    /**
     * @brief Checks if a scene is currently loaded.
     * 
     * @param sceneName Name of the scene to check.
     * @return true if the scene is loaded, false otherwise.
     */
    bool isSceneLoaded(const std::string &sceneName);

    // in the future: bool preloadScene/unloadScene/isSceneLoaded(const Scene &scene); with gamecoe::Scene object!

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
     * @param filename Name of the sound file to play (should be in a loaded <general or scene>/{soundSubdir}/ subdirectory).
     * @param volume Volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the sound. Default is false.
     * @param priority Sound priority for resource allocation. Default is Medium.
     * @return SoundHandle to control the playing sound, or INVALID_SOUND_HANDLE (equal to 0) if playback failed.
     */
    SoundHandle playSound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                 SoundPriority priority = SoundPriority::Medium);

    /**
     * @brief Plays a 3D positioned sound with spatial audio properties.
     *
     * @param filename Name of the sound file to play (should be in a loaded <general or scene>/{soundSubdir}/ subdirectory).
     * @param position 3D world position of the sound source.
     * @param velocity 3D velocity vector for doppler effect. Default is zero.
     * @param volume Volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the sound. Default is false.
     * @param priority Sound priority for resource allocation. Default is Medium.
     * @return SoundHandle to control the playing sound, or INVALID_SOUND_HANDLE (equal to 0) if playback failed.
     */
    SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity = Vec3::zero(),
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   SoundPriority priority = SoundPriority::Medium);

    /**
     * @brief Plays a music file with specified properties.
     *
     * @param filename Name of the music file to play (should be in a loaded <general or scene>/{musicSubdir}/ subdirectory).
     * @param volume Volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the music. Default is true.
     * @param priority Music priority for resource allocation. Default is Critical.
     * @return MusicHandle to control the playing music, or INVALID_MUSIC_HANDLE (equal to 0) if playback failed.
     */
    MusicHandle playMusic(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                 SoundPriority priority = SoundPriority::Critical);

    /**
     * @brief Pauses a specific sound.
     * 
     * @param handle Handle of the sound to pause.
     * @return true if successfully paused, false if handle is invalid or sound not playing.
     */
    bool pauseSound(SoundHandle handle);

    /**
     * @brief Pauses a specific music track.
     * 
     * @param handle Handle of the music to pause.
     * @return true if successfully paused, false if handle is invalid or music not playing.
     */
    bool pauseMusic(MusicHandle handle);

    /**
     * @brief Pauses all currently playing sounds.
     */
    void pauseAllSounds();

    /**
     * @brief Pauses all currently playing music tracks.
     */
    void pauseAllMusic();

    /**
     * @brief Pauses all currently playing sounds and music.
     */
    void pauseAll();

    /**
     * @brief Resumes a paused sound.
     * 
     * @param handle Handle of the sound to resume.
     * @return true if successfully resumed, false if handle is invalid or sound not paused.
     */
    bool resumeSound(SoundHandle handle);

    /**
     * @brief Resumes a paused music track.
     * 
     * @param handle Handle of the music to resume.
     * @return true if successfully resumed, false if handle is invalid or music not paused.
     */
    bool resumeMusic(MusicHandle handle);

    /**
     * @brief Resumes all paused sounds.
     */
    void resumeAllSounds();

    /**
     * @brief Resumes all paused music tracks.
     */
    void resumeAllMusic();

    /**
     * @brief Resumes all paused sounds and music.
     */
    void resumeAll();

    /**
     * @brief Stops a playing or paused sound.
     * 
     * @param handle Handle of the sound to stop.
     * @return true if successfully stopped, false if handle is invalid.
     */
    bool stopSound(SoundHandle handle);

    /**
     * @brief Stops a playing or paused music track.
     * 
     * @param handle Handle of the music to stop.
     * @return true if successfully stopped, false if handle is invalid.
     */
    bool stopMusic(MusicHandle handle);

    /**
     * @brief Stops all currently active sounds.
     */
    void stopAllSounds();

    /**
     * @brief Stops all currently active music tracks.
     */
    void stopAllMusic();

    /**
     * @brief Stops all currently active sounds and music.
     */
    void stopAll();

    /**
     * @brief Sets the volume of a specific sound.
     * 
     * @param handle Handle of the sound to modify.
     * @param volume New volume level.
     * @return true if successfully set, false if handle is invalid.
     */
    bool setSoundVolume(SoundHandle handle, float volume);

    /**
     * @brief Sets the volume of a specific music track.
     * 
     * @param handle Handle of the music to modify.
     * @param volume New volume level.
     * @return true if successfully set, false if handle is invalid.
     */
    bool setMusicVolume(MusicHandle handle, float volume);

    /**
     * @brief Sets the pitch of a specific sound.
     * 
     * @param handle Handle of the sound to modify.
     * @param pitch New pitch multiplier.
     * @return true if successfully set, false if handle is invalid.
     */
    bool setSoundPitch(SoundHandle handle, float pitch);

    /**
     * @brief Sets the pitch of a specific music track.
     * 
     * @param handle Handle of the music to modify.
     * @param pitch New pitch multiplier.
     * @return true if successfully set, false if handle is invalid.
     */
    bool setMusicPitch(MusicHandle handle, float pitch);

    /**
     * @brief Sets the 3D position of a sound source.
     * 
     * @param handle Handle of the sound to modify.
     * @param position New 3D world position.
     * @return true if successfully set, false if handle is invalid.
     */
    bool setSoundPosition(SoundHandle handle, const Vec3 &position);

    /**
     * @brief Sets the 3D velocity of a sound source for doppler effect.
     * 
     * @param handle Handle of the sound to modify.
     * @param velocity New 3D velocity vector.
     * @return true if successfully set, false if handle is invalid.
     */
    bool setSoundVelocity(SoundHandle handle, const Vec3 &velocity);

    /**
     * @brief Checks if a sound is currently playing.
     * 
     * @param handle Handle of the sound to check.
     * @return true if sound is playing, false if paused, stopped, or handle is invalid.
     */
    bool isSoundPlaying(SoundHandle handle);

    /**
     * @brief Checks if a music track is currently playing.
     * 
     * @param handle Handle of the music to check.
     * @return true if music is playing, false if paused, stopped, or handle is invalid.
     */
    bool isMusicPlaying(MusicHandle handle);

    /**
     * @brief Checks if a sound is currently paused.
     * 
     * @param handle Handle of the sound to check.
     * @return true if sound is paused, false if playing, stopped, or handle is invalid.
     */
    bool isSoundPaused(SoundHandle handle);

    /**
     * @brief Checks if a music track is currently paused.
     * 
     * @param handle Handle of the music to check.
     * @return true if music is paused, false if playing, stopped, or handle is invalid.
     */
    bool isMusicPaused(MusicHandle handle);

    /**
     * @brief Checks if a sound is currently stopped.
     * 
     * @param handle Handle of the sound to check.
     * @return true if sound is stopped, false if playing, paused, or handle is invalid.
     */
    bool isSoundStopped(SoundHandle handle);

    /**
     * @brief Checks if a music track is currently stopped.
     * 
     * @param handle Handle of the music to check.
     * @return true if music is stopped, false if playing, paused, or handle is invalid.
     */
    bool isMusicStopped(MusicHandle handle);

    /**
     * @brief Gets the number of currently active sound sources.
     * 
     * @return Number of sounds that are active.
     */
    size_t getActiveSoundsCount();

    /**
     * @brief Gets the number of currently active music tracks.
     * 
     * @return Number of music tracks that are active.
     */
    size_t getActiveMusicCount();

    /**
     * @brief Plays a sound with a fade-in effect from silence to target volume.
     *
     * @param filename Name of the sound file to play (relative to loaded sound effects directories).
     * @param duration Fade-in duration in seconds.
     * @param volume Target volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the sound. Default is false.
     * @param priority Sound priority for resource allocation. Default is Medium.
     * @return SoundHandle to control the playing sound, or INVALID_SOUND_HANDLE (equal to 0) if playback failed.
     */
    SoundHandle fadeInSound(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   SoundPriority priority = SoundPriority::Medium);

    /**
     * @brief Plays music with a fade-in effect from silence to target volume.
     *
     * @param filename Name of the music file to play (relative to loaded music directories).
     * @param duration Fade-in duration in seconds.
     * @param volume Target volume level. Default is 1.0.
     * @param pitch Pitch multiplier. Default is 1.0.
     * @param loop Whether to loop the music. Default is true.
     * @param priority Music priority for resource allocation. Default is Critical.
     * @return MusicHandle to control the playing music, or INVALID_MUSIC_HANDLE (equal to 0) if playback failed.
     */
    MusicHandle fadeInMusic(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                   SoundPriority priority = SoundPriority::Critical);

    /**
     * @brief Fades out a sound from current volume to silence, then stops it.
     * 
     * @param handle Handle of the sound to fade out.
     * @param duration Fade-out duration in seconds.
     * @return true if fade started successfully, false if handle is invalid.
     */
    bool fadeOutSound(SoundHandle handle, float duration);

    /**
     * @brief Fades out music from current volume to silence, then stops it.
     * 
     * @param handle Handle of the music to fade out.
     * @param duration Fade-out duration in seconds.
     * @return true if fade started successfully, false if handle is invalid.
     */
    bool fadeOutMusic(MusicHandle handle, float duration);

    /**
     * @brief Fades a sound from current volume to a target volume over time.
     * 
     * @param handle Handle of the sound to fade.
     * @param targetVolume Target volume level.
     * @param duration Fade duration in seconds.
     * @return true if fade started successfully, false if handle is invalid.
     */
    bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration);

    /**
     * @brief Fades music from current volume to a target volume over time.
     * 
     * @param handle Handle of the music to fade.
     * @param targetVolume Target volume level.
     * @param duration Fade duration in seconds.
     * @return true if fade started successfully, false if handle is invalid.
     */
    bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration);

    /**
     * @brief Sets the master volume multiplier for all audio (sounds and music).
     * 
     * @param volume Master volume multiplier level.
     * @return true if successfully set, false on error.
     */
    bool setMasterVolume(float volume);

    /**
     * @brief Sets the master volume multiplier for all sound effects.
     * 
     * @param volume Sounds master volume level multiplier.
     * @return true if successfully set, false on error.
     */
    bool setMasterSoundsVolume(float volume);

    /**
     * @brief Sets the master volume multiplier for all music tracks.
     * 
     * @param volume Music master volume level multiplier.
     * @return true if successfully set, false on error.
     */
    bool setMasterMusicVolume(float volume);

    /**
     * @brief Sets the master pitch multiplier for all audio (sounds and music).
     * 
     * @param pitch Master pitch multiplier.
     * @return true if successfully set, false on error.
     */
    bool setMasterPitch(float pitch);

    /**
     * @brief Sets the master pitch multiplier for all sounds effects.
     * 
     * @param pitch Sounds master pitch multiplier.
     * @return true if successfully set, false on error.
     */
    bool setMasterSoundsPitch(float pitch);

    /**
     * @brief Sets the master pitch multiplier for all music tracks.
     * 
     * @param pitch Music master pitch multiplier.
     * @return true if successfully set, false on error.
     */
    bool setMasterMusicPitch(float pitch);

    /**
     * @brief Gets the current master volume multiplier for all audio.
     *
     * @return Current master volume level multiplier.
     */
    float getMasterVolume();

    /**
     * @brief Gets the current master volume multiplier for sound effects.
     *
     * @return Current sounds master volume level multiplier.
     */
    float getMasterSoundsVolume();

    /**
     * @brief Gets the current master volume multiplier for music tracks.
     *
     * @return Current music master volume level multiplier.
     */
    float getMasterMusicVolume();

    /**
     * @brief Gets the current master pitch multiplier for all audio.
     *
     * @return Current master pitch multiplier.
     */
    float getMasterPitch();

    /**
     * @brief Gets the current master pitch multiplier for sound effects.
     *
     * @return Current sounds master pitch multiplier.
     */
    float getMasterSoundsPitch();

    /**
     * @brief Gets the current master pitch multiplier for music tracks.
     *
     * @return Current music master pitch multiplier.
     */
    float getMasterMusicPitch();

    /**
     * @brief Mutes all sound effects while preserving their volume settings.
     */
    void muteAllSounds();

    /**
     * @brief Mutes all music tracks while preserving their volume settings.
     */
    void muteAllMusic();

    /**
     * @brief Mutes all audio (sounds and music) while preserving volume settings.
     */
    void muteAll();

    /**
     * @brief Unmutes all sound effects, restoring their previous volume settings.
     */
    void unmuteAllSounds();

    /**
     * @brief Unmutes all music, restoring their previous volume settings.
     */
    void unmuteAllMusic();

    /**
     * @brief Unmutes all audio (sounds and music), restoring previous volume settings.
     */
    void unmuteAll();

    /**
     * @brief Checks if all audio is currently muted.
     * 
     * @return true if all audio is muted, false otherwise.
     */
    bool isMuted();

    /**
     * @brief Checks if all sound effects are currently muted.
     * 
     * @return true if sounds are muted, false otherwise.
     */
    bool isSoundsMuted();

    /**
     * @brief Checks if all music tracks are currently muted.
     * 
     * @return true if music is muted, false otherwise.
     */
    bool isMusicMuted();

    /**
     * @brief Updates all 3D audio listener properties at once.
     * 
     * @param position 3D world position of the listener.
     * @param velocity 3D velocity vector for doppler effect.
     * @param forward Forward direction vector (must be normalized).
     * @param up Up direction vector (must be normalized). Default is Vec3::up().
     * @return true if successfully updated, false on error.
     */
    bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up = Vec3::up());

    /**
     * @brief Sets the 3D position of the audio listener.
     * 
     * @param position 3D world position of the listener.
     * @return true if successfully set, false on error.
     */
    bool setListenerPosition(const Vec3 &position);

    /**
     * @brief Sets the velocity of the audio listener for doppler effect.
     * 
     * @param velocity 3D velocity vector of the listener.
     * @return true if successfully set, false on error.
     */
    bool setListenerVelocity(const Vec3 &velocity);

    /**
     * @brief Sets the forward direction of the audio listener.
     * 
     * @param forward Forward direction vector (must be normalized).
     * @return true if successfully set, false on error.
     */
    bool setListenerForward(const Vec3 &forward);

    /**
     * @brief Sets the up direction of the audio listener.
     * 
     * @param up Up direction vector (must be normalized).
     * @return true if successfully set, false on error.
     */
    bool setListenerUp(const Vec3 &up);

    /**
     * @brief Gets the current 3D position of the audio listener.
     * 
     * @return Current listener position.
     */
    Vec3 getListenerPosition();

    /**
     * @brief Gets the current velocity of the audio listener.
     * 
     * @return Current listener velocity vector.
     */
    Vec3 getListenerVelocity();

    /**
     * @brief Gets the current forward direction of the audio listener.
     * 
     * @return Current listener forward direction vector.
     */
    Vec3 getListenerForward();

    /**
     * @brief Gets the current up direction of the audio listener.
     * 
     * @return Current listener up direction vector.
     */
    Vec3 getListenerUp();

    /**
     * @brief Gets the last error message and clears the error state.
     * 
     * @return Last error message, or empty string if no error occurred.
     */
    const std::string getError();

    /**
     * @brief Clears the current error state without returning the message.
     */
    void clearError();

    /**
     * @brief Checks if a handle (sound or music) is valid and active.
     * 
     * @param handle Handle to validate.
     * @return true if handle is valid and represents active audio, false otherwise.
     */
    bool isHandleValid(size_t handle);
} // namespace soundcoe