#pragma once

#include <soundcoe/playback/sound_manager.hpp>
#include <soundcoe/utils/math.hpp>
#include <logcoe.hpp>

namespace soundcoe
{
    namespace detail
    {
        SoundManager& getSoundManagerInstance();
    }
}

namespace soundcoe
{
    static bool initialize(const std::string &audioRootDirectory, size_t maxSources = 32,
                           size_t maxCacheSizeMB = 64, const std::string &soundSubdir = "sfx",
                           const std::string &musicSubdir = "music", LogLevel level = LogLevel::INFO)
    {
        return detail::getSoundManagerInstance().initialize(audioRootDirectory, maxSources, maxCacheSizeMB,
                                                                   soundSubdir, musicSubdir, level);
    }

    static void shutdown()
    {
        detail::getSoundManagerInstance().shutdown();
    }

    static bool isInitialized()
    {
        return detail::getSoundManagerInstance().isInitialized();
    }

    static bool preloadScene(const std::string &sceneName)
    {
        return detail::getSoundManagerInstance().preloadScene(sceneName);
    }

    static bool unloadScene(const std::string &sceneName)
    {
        return detail::getSoundManagerInstance().unloadScene(sceneName);
    }

    static bool isSceneLoaded(const std::string &sceneName)
    {
        return detail::getSoundManagerInstance().isSceneLoaded(sceneName);
    }

    // in the future: static bool preloadScene/unloadScene/isSceneLoaded(const Scene &scene); with gamecoe::Scene object!

    static void update()
    {
        detail::getSoundManagerInstance().update();
    }

    static SoundHandle playSound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                 SoundPriority priority = SoundPriority::Medium)
    {
        return detail::getSoundManagerInstance().playSound(filename, volume, pitch, loop, priority);
    }

    static SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity = Vec3::zero(),
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   SoundPriority priority = SoundPriority::Medium)
    {
        return detail::getSoundManagerInstance().playSound3D(filename, position, velocity, volume, pitch, loop, priority);
    }

    static MusicHandle playMusic(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                 SoundPriority priority = SoundPriority::Critical)
    {
        return detail::getSoundManagerInstance().playMusic(filename, volume, pitch, loop, priority);
    }

    static bool pauseSound(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().pauseSound(handle);
    }

    static bool pauseMusic(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().pauseMusic(handle);
    }

    static bool pauseAllSounds()
    {
        return detail::getSoundManagerInstance().pauseAllSounds();
    }

    static bool pauseAllMusic()
    {
        return detail::getSoundManagerInstance().pauseAllMusic();
    }

    static bool pauseAll()
    {
        return detail::getSoundManagerInstance().pauseAll();
    }

    static bool resumeSound(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().resumeSound(handle);
    }

    static bool resumeMusic(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().resumeMusic(handle);
    }

    static bool resumeAllSounds()
    {
        return detail::getSoundManagerInstance().resumeAllSounds();
    }

    static bool resumeAllMusic()
    {
        return detail::getSoundManagerInstance().resumeAllMusic();
    }

    static bool resumeAll()
    {
        return detail::getSoundManagerInstance().resumeAll();
    }

    static bool stopSound(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().stopSound(handle);
    }

    static bool stopMusic(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().stopMusic(handle);
    }

    static bool stopAllSounds()
    {
        return detail::getSoundManagerInstance().stopAllSounds();
    }

    static bool stopAllMusic()
    {
        return detail::getSoundManagerInstance().stopAllMusic();
    }

    static bool stopAll()
    {
        return detail::getSoundManagerInstance().stopAll();
    }

    static bool setSoundVolume(SoundHandle handle, float volume)
    {
        return detail::getSoundManagerInstance().setSoundVolume(handle, volume);
    }

    static bool setMusicVolume(MusicHandle handle, float volume)
    {
        return detail::getSoundManagerInstance().setMusicVolume(handle, volume);
    }

    static bool setSoundPitch(SoundHandle handle, float pitch)
    {
        return detail::getSoundManagerInstance().setSoundPitch(handle, pitch);
    }

    static bool setMusicPitch(MusicHandle handle, float pitch)
    {
        return detail::getSoundManagerInstance().setMusicPitch(handle, pitch);
    }

    static bool setSoundPosition(SoundHandle handle, const Vec3 &position)
    {
        return detail::getSoundManagerInstance().setSoundPosition(handle, position);
    }

    static bool setSoundVelocity(SoundHandle handle, const Vec3 &velocity)
    {
        return detail::getSoundManagerInstance().setSoundVelocity(handle, velocity);
    }

    static bool isSoundPlaying(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().isSoundPlaying(handle);
    }

    static bool isMusicPlaying(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().isMusicPlaying(handle);
    }

    static bool isSoundPaused(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().isSoundPaused(handle);
    }

    static bool isMusicPaused(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().isMusicPaused(handle);
    }

    static bool isSoundStopped(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().isSoundStopped(handle);
    }

    static bool isMusicStopped(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().isMusicStopped(handle);
    }

    static size_t getActiveSoundsCount()
    {
        return detail::getSoundManagerInstance().getActiveSoundsCount();
    }

    static size_t getActiveMusicCount()
    {
        return detail::getSoundManagerInstance().getActiveMusicCount();
    }

    static SoundHandle fadeInSound(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   SoundPriority priority = SoundPriority::Medium)
    {
        return detail::getSoundManagerInstance().fadeInSound(filename, duration, volume, pitch, loop, priority);
    }

    static MusicHandle fadeInMusic(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                   SoundPriority priority = SoundPriority::Critical)
    {
        return detail::getSoundManagerInstance().fadeInMusic(filename, duration, volume, pitch, loop, priority);
    }

    static bool fadeOutSound(SoundHandle handle, float duration)
    {
        return detail::getSoundManagerInstance().fadeOutSound(handle, duration);
    }

    static bool fadeOutMusic(MusicHandle handle, float duration)
    {
        return detail::getSoundManagerInstance().fadeOutMusic(handle, duration);
    }

    static bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration)
    {
        return detail::getSoundManagerInstance().fadeToVolumeSound(handle, targetVolume, duration);
    }

    static bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration)
    {
        return detail::getSoundManagerInstance().fadeToVolumeMusic(handle, targetVolume, duration);
    }

    static bool setMasterVolume(float volume)
    {
        return detail::getSoundManagerInstance().setMasterVolume(volume);
    }

    static bool setMasterSoundsVolume(float volume)
    {
        return detail::getSoundManagerInstance().setMasterSoundsVolume(volume);
    }

    static bool setMasterMusicVolume(float volume)
    {
        return detail::getSoundManagerInstance().setMasterMusicVolume(volume);
    }

    static bool setMasterPitch(float pitch)
    {
        return detail::getSoundManagerInstance().setMasterPitch(pitch);
    }

    static bool setMasterSoundsPitch(float pitch)
    {
        return detail::getSoundManagerInstance().setMasterSoundsPitch(pitch);
    }

    static bool setMasterMusicPitch(float pitch)
    {
        return detail::getSoundManagerInstance().setMasterMusicPitch(pitch);
    }

    static float getMasterVolume()
    {
        return detail::getSoundManagerInstance().getMasterVolume();
    }

    static float getMasterSoundsVolume()
    {
        return detail::getSoundManagerInstance().getMasterSoundsVolume();
    }

    static float getMasterMusicVolume()
    {
        return detail::getSoundManagerInstance().getMasterMusicVolume();
    }

    static float getMasterPitch()
    {
        return detail::getSoundManagerInstance().getMasterPitch();
    }

    static float getMasterSoundsPitch()
    {
        return detail::getSoundManagerInstance().getMasterSoundsPitch();
    }

    static float getMasterMusicPitch()
    {
        return detail::getSoundManagerInstance().getMasterMusicPitch();
    }

    static bool muteAllSounds()
    {
        return detail::getSoundManagerInstance().muteAllSounds();
    }

    static bool muteAllMusic()
    {
        return detail::getSoundManagerInstance().muteAllMusic();
    }

    static bool muteAll()
    {
        return detail::getSoundManagerInstance().muteAll();
    }

    static bool unmuteAllSounds()
    {
        return detail::getSoundManagerInstance().unmuteAllSounds();
    }

    static bool unmuteAllMusic()
    {
        return detail::getSoundManagerInstance().unmuteAllMusic();
    }

    static bool unmuteAll()
    {
        return detail::getSoundManagerInstance().unmuteAll();
    }

    static bool isMuted()
    {
        return detail::getSoundManagerInstance().isMuted();
    }

    static bool isSoundsMuted()
    {
        return detail::getSoundManagerInstance().isSoundsMuted();
    }

    static bool isMusicMuted()
    {
        return detail::getSoundManagerInstance().isMusicMuted();
    }

    static bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up = Vec3::up())
    {
        return detail::getSoundManagerInstance().updateListener(position, velocity, forward, up);
    }

    static bool setListenerPosition(const Vec3 &position)
    {
        return detail::getSoundManagerInstance().setListenerPosition(position);
    }

    static bool setListenerVelocity(const Vec3 &velocity)
    {
        return detail::getSoundManagerInstance().setListenerVelocity(velocity);
    }

    static bool setListenerForward(const Vec3 &forward)
    {
        return detail::getSoundManagerInstance().setListenerForward(forward);
    }

    static bool setListenerUp(const Vec3 &up)
    {
        return detail::getSoundManagerInstance().setListenerUp(up);
    }

    static Vec3 getListenerPosition()
    {
        return detail::getSoundManagerInstance().getListenerPosition();
    }

    static Vec3 getListenerVelocity()
    {
        return detail::getSoundManagerInstance().getListenerVelocity();
    }

    static Vec3 getListenerForward()
    {
        return detail::getSoundManagerInstance().getListenerForward();
    }

    static Vec3 getListenerUp()
    {
        return detail::getSoundManagerInstance().getListenerUp();
    }

    static const std::string getError()
    {
        return detail::getSoundManagerInstance().getError();
    }

    static void clearError()
    {
        detail::getSoundManagerInstance().clearError();
    }

    static bool isHandleValid(size_t handle)
    {
        return detail::getSoundManagerInstance().isHandleValid(handle);
    }
} // namespace soundcoe