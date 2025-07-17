#pragma once

#include <soundcoe/playback/sound_manager.hpp>

namespace soundcoe
{
    static detail::SoundManager s_soundManager;

    static bool initialize(const std::string &audioRootDirectory, size_t maxSources = 32, size_t maxCacheSizeMB = 64,
                           LogLevel level = LogLevel::INFO)
    {
        return s_soundManager.initialize(audioRootDirectory, maxSources, maxCacheSizeMB, level);
    }

    static void shutdown()
    {
        s_soundManager.shutdown();
    }

    static bool isInitialized()
    {
        return s_soundManager.isInitialized();
    }

    static bool preloadScene(const std::string &sceneName)
    {
        return s_soundManager.preloadScene(sceneName);
    }

    static bool unloadScene(const std::string &sceneName)
    {
        return s_soundManager.unloadScene(sceneName);
    }

    static bool isSceneLoaded(const std::string &sceneName)
    {
        return s_soundManager.isSceneLoaded(sceneName);
    }

    // in the future: static bool preloadScene/unloadScene/isSceneLoaded(const Scene &scene); with gamecoe::Scene object!

    static void update()
    {
        s_soundManager.update();
    }

    static SoundHandle playSound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                 SoundPriority priority = SoundPriority::Medium)
    {
        return s_soundManager.playSound(filename, volume, pitch, loop, priority);
    }

    static SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity = Vec3::zero(),
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   SoundPriority priority = SoundPriority::Medium)
    {
        return s_soundManager.playSound3D(filename, position, velocity, volume, pitch, loop, priority);
    }

    static MusicHandle playMusic(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                 SoundPriority priority = SoundPriority::Critical)
    {
        return s_soundManager.playMusic(filename, volume, pitch, loop, priority);
    }

    static bool pauseSound(SoundHandle handle)
    {
        return s_soundManager.pauseSound(handle);
    }

    static bool pauseMusic(MusicHandle handle)
    {
        return s_soundManager.pauseMusic(handle);
    }

    static bool pauseAllSounds()
    {
        return s_soundManager.pauseAllSounds();
    }

    static bool pauseAllMusic()
    {
        return s_soundManager.pauseAllMusic();
    }

    static bool pauseAll()
    {
        return s_soundManager.pauseAll();
    }

    static bool resumeSound(SoundHandle handle)
    {
        return s_soundManager.resumeSound(handle);
    }

    static bool resumeMusic(MusicHandle handle)
    {
        return s_soundManager.resumeMusic(handle);
    }

    static bool resumeAllSounds()
    {
        return s_soundManager.resumeAllSounds();
    }

    static bool resumeAllMusic()
    {
        return s_soundManager.resumeAllMusic();
    }

    static bool resumeAll()
    {
        return s_soundManager.resumeAll();
    }

    static bool stopSound(SoundHandle handle)
    {
        return s_soundManager.stopSound(handle);
    }

    static bool stopMusic(MusicHandle handle)
    {
        return s_soundManager.stopMusic(handle);
    }

    static bool stopAllSounds()
    {
        return s_soundManager.stopAllSounds();
    }

    static bool stopAllMusic()
    {
        return s_soundManager.stopAllMusic();
    }

    static bool stopAll()
    {
        return s_soundManager.stopAll();
    }

    static bool setSoundVolume(SoundHandle handle, float volume)
    {
        return s_soundManager.setSoundVolume(handle, volume);
    }

    static bool setMusicVolume(MusicHandle handle, float volume)
    {
        return s_soundManager.setMusicVolume(handle, volume);
    }

    static bool setSoundPitch(SoundHandle handle, float pitch)
    {
        return s_soundManager.setSoundPitch(handle, pitch);
    }

    static bool setMusicPitch(MusicHandle handle, float pitch)
    {
        return s_soundManager.setMusicPitch(handle, pitch);
    }

    static bool setSoundPosition(SoundHandle handle, const Vec3 &position)
    {
        return s_soundManager.setSoundPosition(handle, position);
    }

    static bool setSoundVelocity(SoundHandle handle, const Vec3 &velocity)
    {
        return s_soundManager.setSoundVelocity(handle, velocity);
    }

    static bool isSoundPlaying(SoundHandle handle)
    {
        return s_soundManager.isSoundPlaying(handle);
    }

    static bool isMusicPlaying(MusicHandle handle)
    {
        return s_soundManager.isMusicPlaying(handle);
    }

    static bool isSoundPaused(SoundHandle handle)
    {
        return s_soundManager.isSoundPaused(handle);
    }

    static bool isMusicPaused(MusicHandle handle)
    {
        return s_soundManager.isMusicPaused(handle);
    }

    static bool isSoundStopped(SoundHandle handle)
    {
        return s_soundManager.isSoundStopped(handle);
    }

    static bool isMusicStopped(MusicHandle handle)
    {
        return s_soundManager.isMusicStopped(handle);
    }

    static size_t getActiveSoundsCount()
    {
        return s_soundManager.getActiveSoundsCount();
    }

    static size_t getActiveMusicCount()
    {
        return s_soundManager.getActiveMusicCount();
    }

    static SoundHandle fadeInSound(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   SoundPriority priority = SoundPriority::Medium)
    {
        return s_soundManager.fadeInSound(filename, duration, volume, pitch, loop, priority);
    }

    static MusicHandle fadeInMusic(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                   SoundPriority priority = SoundPriority::Critical)
    {
        return s_soundManager.fadeInMusic(filename, duration, volume, pitch, loop, priority);
    }

    static bool fadeOutSound(SoundHandle handle, float duration)
    {
        return s_soundManager.fadeOutSound(handle, duration);
    }

    static bool fadeOutMusic(MusicHandle handle, float duration)
    {
        return s_soundManager.fadeOutMusic(handle, duration);
    }

    static bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration)
    {
        return s_soundManager.fadeToVolumeSound(handle, targetVolume, duration);
    }

    static bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration)
    {
        return s_soundManager.fadeToVolumeMusic(handle, targetVolume, duration);
    }

    static bool setMasterVolume(float volume)
    {
        return s_soundManager.setMasterVolume(volume);
    }

    static bool setMasterSoundsVolume(float volume)
    {
        return s_soundManager.setMasterSoundsVolume(volume);
    }

    static bool setMasterMusicVolume(float volume)
    {
        return s_soundManager.setMasterMusicVolume(volume);
    }

    static bool setMasterPitch(float pitch)
    {
        return s_soundManager.setMasterPitch(pitch);
    }

    static bool setMasterSoundsPitch(float pitch)
    {
        return s_soundManager.setMasterSoundsPitch(pitch);
    }

    static bool setMasterMusicPitch(float pitch)
    {
        return s_soundManager.setMasterMusicPitch(pitch);
    }

    static float getMasterVolume()
    {
        return s_soundManager.getMasterVolume();
    }

    static float getMasterSoundsVolume()
    {
        return s_soundManager.getMasterSoundsVolume();
    }

    static float getMasterMusicVolume()
    {
        return s_soundManager.getMasterMusicVolume();
    }

    static float getMasterPitch()
    {
        return s_soundManager.getMasterPitch();
    }

    static float getMasterSoundsPitch()
    {
        return s_soundManager.getMasterSoundsPitch();
    }

    static float getMasterMusicPitch()
    {
        return s_soundManager.getMasterMusicPitch();
    }

    static bool muteAllSounds()
    {
        return s_soundManager.muteAllSounds();
    }

    static bool muteAllMusic()
    {
        return s_soundManager.muteAllMusic();
    }

    static bool muteAll()
    {
        return s_soundManager.muteAll();
    }

    static bool unmuteAllSounds()
    {
        return s_soundManager.unmuteAllSounds();
    }

    static bool unmuteAllMusic()
    {
        return s_soundManager.unmuteAllMusic();
    }

    static bool unmuteAll()
    {
        return s_soundManager.unmuteAll();
    }

    static bool isMuted()
    {
        return s_soundManager.isMuted();
    }

    static bool isSoundsMuted()
    {
        return s_soundManager.isSoundsMuted();
    }

    static bool isMusicMuted()
    {
        return s_soundManager.isMusicMuted();
    }

    static bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up = Vec3::up())
    {
        return s_soundManager.updateListener(position, velocity, forward, up);
    }

    static bool setListenerPosition(const Vec3 &position)
    {
        return s_soundManager.setListenerPosition(position);
    }

    static bool setListenerVelocity(const Vec3 &velocity)
    {
        return s_soundManager.setListenerVelocity(velocity);
    }

    static bool setListenerForward(const Vec3 &forward)
    {
        return s_soundManager.setListenerForward(forward);
    }

    static bool setListenerUp(const Vec3 &up)
    {
        return s_soundManager.setListenerUp(up);
    }

    static Vec3 getListenerPosition()
    {
        return s_soundManager.getListenerPosition();
    }

    static Vec3 getListenerVelocity()
    {
        return s_soundManager.getListenerVelocity();
    }

    static Vec3 getListenerForward()
    {
        return s_soundManager.getListenerForward();
    }

    static Vec3 getListenerUp()
    {
        return s_soundManager.getListenerUp();
    }

    static const std::string getError()
    {
        return s_soundManager.getError();
    }

    static void clearError()
    {
        s_soundManager.clearError();
    }

    static bool isHandleValid(size_t handle)
    {
        return s_soundManager.isHandleValid(handle);
    }
} // namespace soundcoe