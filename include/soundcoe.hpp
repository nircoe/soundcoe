#pragma once

#include <soundcoe/playback/sound_manager.hpp>

namespace soundcoe
{
    static bool initialize(const std::string &audioRootDirectory, size_t maxSources = 32, size_t maxCacheSizeMB = 64,
                           LogLevel level = LogLevel::INFO);
    static void shutdown();
    static bool isInitialized();

    static bool preloadScene(const std::string &sceneName);
    static bool unloadScene(const std::string &sceneName);
    static bool isSceneLoaded(const std::string &sceneName);
    // in the future: static bool preloadScene/unloadScene/isSceneLoaded(const Scene &scene); with gamecoe::Scene object!

    static void update();

    static SoundHandle playSound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                 SoundPriority priority = SoundPriority::Medium);
    static SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity = Vec3::zero(),
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   SoundPriority priority = SoundPriority::Medium);
    static MusicHandle playMusic(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                 SoundPriority priority = SoundPriority::Critical);

    static bool pauseSound(SoundHandle handle);
    static bool pauseMusic(MusicHandle handle);
    static bool pauseAllSounds();
    static bool pauseAllMusic();
    static bool pauseAll();
    static bool resumeSound(SoundHandle handle);
    static bool resumeMusic(MusicHandle handle);
    static bool resumeAllSounds();
    static bool resumeAllMusic();
    static bool resumeAll();
    static bool stopSound(SoundHandle handle);
    static bool stopMusic(MusicHandle handle);
    static bool stopAllSounds();
    static bool stopAllMusic();
    static bool stopAll();
    static bool setSoundVolume(SoundHandle handle, float volume);
    static bool setMusicVolume(MusicHandle handle, float volume);
    static bool setSoundPitch(SoundHandle handle, float pitch);
    static bool setMusicPitch(MusicHandle handle, float pitch);
    static bool setSoundPosition(SoundHandle handle, const Vec3 &position);
    static bool setSoundVelocity(SoundHandle handle, const Vec3 &velocity);

    static bool isSoundPlaying(SoundHandle handle);
    static bool isMusicPlaying(MusicHandle handle);
    static bool isSoundPaused(SoundHandle handle);
    static bool isMusicPaused(MusicHandle handle);
    static bool isSoundStopped(SoundHandle handle);
    static bool isMusicStopped(MusicHandle handle);

    static size_t getActiveSoundsCount();
    static size_t getActiveMusicCount();

    static SoundHandle fadeInSound(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                   SoundPriority priority = SoundPriority::Medium);
    static MusicHandle fadeInMusic(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                   SoundPriority priority = SoundPriority::Critical);
    static bool fadeOutSound(SoundHandle handle, float duration);
    static bool fadeOutMusic(MusicHandle handle, float duration);
    static bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration);
    static bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration);

    static bool setMasterVolume(float volume);
    static bool setMasterSoundsVolume(float volume);
    static bool setMasterMusicVolume(float volume);
    static bool setMasterPitch(float pitch);
    static bool setMasterSoundsPitch(float pitch);
    static bool setMasterMusicPitch(float pitch);
    static float getMasterVolume();
    static float getMasterSoundsVolume();
    static float getMasterMusicVolume();
    static float getMasterPitch();
    static float getMasterSoundsPitch();
    static float getMasterMusicPitch();

    static bool muteAllSounds();
    static bool muteAllMusic();
    static bool muteAll();
    static bool unmuteAllSounds();
    static bool unmuteAllMusic();
    static bool unmuteAll();
    static bool isMuted();
    static bool isSoundsMuted();
    static bool isMusicMuted();

    static bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up = Vec3::up());
    static bool setListenerPosition(const Vec3 &position);
    static bool setListenerVelocity(const Vec3 &velocity);
    static bool setListenerForward(const Vec3 &forward);
    static bool setListenerUp(const Vec3 &up = Vec3::up());

    static Vec3 getListenerPosition();
    static Vec3 getListenerVelocity();
    static Vec3 getListenerForward();
    static Vec3 getListenerUp();

    static const std::string getError();
    static void clearError();

    static bool isHandleValid(size_t handle);
} // namespace soundcoe