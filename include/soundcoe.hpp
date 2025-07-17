#pragma once

#include <soundcoe/playback/sound_manager.hpp>
#include <soundcoe/utils/math.hpp>
#include <logcoe.hpp>

namespace
{
    soundcoe::detail::SoundManager& getSoundManager()
    {
        static soundcoe::detail::SoundManager s_soundManager;
        logcoe::info("getSoundManager() called, returning reference to static instance");
        return s_soundManager;
    }
}

namespace soundcoe
{
    static bool initialize(const std::string &audioRootDirectory, size_t maxSources = 32,
                           size_t maxCacheSizeMB = 64, const std::string &soundSubdir = "sfx",
                           const std::string &musicSubdir = "music", LogLevel level = LogLevel::INFO)
    {
        logcoe::info("soundcoe::initialize() wrapper called");
        auto& manager = getSoundManager();
        bool result = manager.initialize(audioRootDirectory, maxSources, maxCacheSizeMB,
                                        soundSubdir, musicSubdir, level);
        logcoe::info("soundcoe::initialize() wrapper returning " + std::to_string(result));
        return result;
    }

    static void shutdown()
    {
        getSoundManager().shutdown();
    }

    static bool isInitialized()
    {
        auto& manager = getSoundManager();
        bool result = manager.isInitialized();
        logcoe::info("soundcoe::isInitialized() wrapper called, returning " + std::to_string(result));
        return result;
    }

    static bool preloadScene(const std::string &sceneName)
    {
        return getSoundManager().preloadScene(sceneName);
    }

    static bool unloadScene(const std::string &sceneName)
    {
        return getSoundManager().unloadScene(sceneName);
    }

    static bool isSceneLoaded(const std::string &sceneName)
    {
        return getSoundManager().isSceneLoaded(sceneName);
    }

    // in the future: static bool preloadScene/unloadScene/isSceneLoaded(const Scene &scene); with gamecoe::Scene object!

    static void update()
    {
        getSoundManager().update();
    }

    static SoundHandle playSound(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                 SoundPriority priority = SoundPriority::Medium)
    {
        logcoe::info("soundcoe::playSound() wrapper called for file: " + filename);
        auto& manager = getSoundManager();
        logcoe::info("soundcoe::playSound() got manager reference, checking initialization status");
        if (!manager.isInitialized()) {
            logcoe::error("soundcoe::playSound() called but SoundManager is not initialized!");
            return INVALID_SOUND_HANDLE;
        }
        logcoe::info("soundcoe::playSound() manager is initialized, proceeding with playSound");
        return manager.playSound(filename, volume, pitch, loop, priority);
    }

    static SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity = Vec3::zero(),
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   SoundPriority priority = SoundPriority::Medium)
    {
        return getSoundManager().playSound3D(filename, position, velocity, volume, pitch, loop, priority);
    }

    static MusicHandle playMusic(const std::string &filename, float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                 SoundPriority priority = SoundPriority::Critical)
    {
        return getSoundManager().playMusic(filename, volume, pitch, loop, priority);
    }

    static bool pauseSound(SoundHandle handle)
    {
        return getSoundManager().pauseSound(handle);
    }

    static bool pauseMusic(MusicHandle handle)
    {
        return getSoundManager().pauseMusic(handle);
    }

    static bool pauseAllSounds()
    {
        return getSoundManager().pauseAllSounds();
    }

    static bool pauseAllMusic()
    {
        return getSoundManager().pauseAllMusic();
    }

    static bool pauseAll()
    {
        return getSoundManager().pauseAll();
    }

    static bool resumeSound(SoundHandle handle)
    {
        return getSoundManager().resumeSound(handle);
    }

    static bool resumeMusic(MusicHandle handle)
    {
        return getSoundManager().resumeMusic(handle);
    }

    static bool resumeAllSounds()
    {
        return getSoundManager().resumeAllSounds();
    }

    static bool resumeAllMusic()
    {
        return getSoundManager().resumeAllMusic();
    }

    static bool resumeAll()
    {
        return getSoundManager().resumeAll();
    }

    static bool stopSound(SoundHandle handle)
    {
        return getSoundManager().stopSound(handle);
    }

    static bool stopMusic(MusicHandle handle)
    {
        return getSoundManager().stopMusic(handle);
    }

    static bool stopAllSounds()
    {
        return getSoundManager().stopAllSounds();
    }

    static bool stopAllMusic()
    {
        return getSoundManager().stopAllMusic();
    }

    static bool stopAll()
    {
        return getSoundManager().stopAll();
    }

    static bool setSoundVolume(SoundHandle handle, float volume)
    {
        return getSoundManager().setSoundVolume(handle, volume);
    }

    static bool setMusicVolume(MusicHandle handle, float volume)
    {
        return getSoundManager().setMusicVolume(handle, volume);
    }

    static bool setSoundPitch(SoundHandle handle, float pitch)
    {
        return getSoundManager().setSoundPitch(handle, pitch);
    }

    static bool setMusicPitch(MusicHandle handle, float pitch)
    {
        return getSoundManager().setMusicPitch(handle, pitch);
    }

    static bool setSoundPosition(SoundHandle handle, const Vec3 &position)
    {
        return getSoundManager().setSoundPosition(handle, position);
    }

    static bool setSoundVelocity(SoundHandle handle, const Vec3 &velocity)
    {
        return getSoundManager().setSoundVelocity(handle, velocity);
    }

    static bool isSoundPlaying(SoundHandle handle)
    {
        return getSoundManager().isSoundPlaying(handle);
    }

    static bool isMusicPlaying(MusicHandle handle)
    {
        return getSoundManager().isMusicPlaying(handle);
    }

    static bool isSoundPaused(SoundHandle handle)
    {
        return getSoundManager().isSoundPaused(handle);
    }

    static bool isMusicPaused(MusicHandle handle)
    {
        return getSoundManager().isMusicPaused(handle);
    }

    static bool isSoundStopped(SoundHandle handle)
    {
        return getSoundManager().isSoundStopped(handle);
    }

    static bool isMusicStopped(MusicHandle handle)
    {
        return getSoundManager().isMusicStopped(handle);
    }

    static size_t getActiveSoundsCount()
    {
        return getSoundManager().getActiveSoundsCount();
    }

    static size_t getActiveMusicCount()
    {
        return getSoundManager().getActiveMusicCount();
    }

    static SoundHandle fadeInSound(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = false,
                                   SoundPriority priority = SoundPriority::Medium)
    {
        return getSoundManager().fadeInSound(filename, duration, volume, pitch, loop, priority);
    }

    static MusicHandle fadeInMusic(const std::string &filename, float duration,
                                   float volume = 1.0f, float pitch = 1.0f, bool loop = true,
                                   SoundPriority priority = SoundPriority::Critical)
    {
        return getSoundManager().fadeInMusic(filename, duration, volume, pitch, loop, priority);
    }

    static bool fadeOutSound(SoundHandle handle, float duration)
    {
        return getSoundManager().fadeOutSound(handle, duration);
    }

    static bool fadeOutMusic(MusicHandle handle, float duration)
    {
        return getSoundManager().fadeOutMusic(handle, duration);
    }

    static bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration)
    {
        return getSoundManager().fadeToVolumeSound(handle, targetVolume, duration);
    }

    static bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration)
    {
        return getSoundManager().fadeToVolumeMusic(handle, targetVolume, duration);
    }

    static bool setMasterVolume(float volume)
    {
        return getSoundManager().setMasterVolume(volume);
    }

    static bool setMasterSoundsVolume(float volume)
    {
        return getSoundManager().setMasterSoundsVolume(volume);
    }

    static bool setMasterMusicVolume(float volume)
    {
        return getSoundManager().setMasterMusicVolume(volume);
    }

    static bool setMasterPitch(float pitch)
    {
        return getSoundManager().setMasterPitch(pitch);
    }

    static bool setMasterSoundsPitch(float pitch)
    {
        return getSoundManager().setMasterSoundsPitch(pitch);
    }

    static bool setMasterMusicPitch(float pitch)
    {
        return getSoundManager().setMasterMusicPitch(pitch);
    }

    static float getMasterVolume()
    {
        return getSoundManager().getMasterVolume();
    }

    static float getMasterSoundsVolume()
    {
        return getSoundManager().getMasterSoundsVolume();
    }

    static float getMasterMusicVolume()
    {
        return getSoundManager().getMasterMusicVolume();
    }

    static float getMasterPitch()
    {
        return getSoundManager().getMasterPitch();
    }

    static float getMasterSoundsPitch()
    {
        return getSoundManager().getMasterSoundsPitch();
    }

    static float getMasterMusicPitch()
    {
        return getSoundManager().getMasterMusicPitch();
    }

    static bool muteAllSounds()
    {
        return getSoundManager().muteAllSounds();
    }

    static bool muteAllMusic()
    {
        return getSoundManager().muteAllMusic();
    }

    static bool muteAll()
    {
        return getSoundManager().muteAll();
    }

    static bool unmuteAllSounds()
    {
        return getSoundManager().unmuteAllSounds();
    }

    static bool unmuteAllMusic()
    {
        return getSoundManager().unmuteAllMusic();
    }

    static bool unmuteAll()
    {
        return getSoundManager().unmuteAll();
    }

    static bool isMuted()
    {
        return getSoundManager().isMuted();
    }

    static bool isSoundsMuted()
    {
        return getSoundManager().isSoundsMuted();
    }

    static bool isMusicMuted()
    {
        return getSoundManager().isMusicMuted();
    }

    static bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up = Vec3::up())
    {
        return getSoundManager().updateListener(position, velocity, forward, up);
    }

    static bool setListenerPosition(const Vec3 &position)
    {
        return getSoundManager().setListenerPosition(position);
    }

    static bool setListenerVelocity(const Vec3 &velocity)
    {
        return getSoundManager().setListenerVelocity(velocity);
    }

    static bool setListenerForward(const Vec3 &forward)
    {
        return getSoundManager().setListenerForward(forward);
    }

    static bool setListenerUp(const Vec3 &up)
    {
        return getSoundManager().setListenerUp(up);
    }

    static Vec3 getListenerPosition()
    {
        return getSoundManager().getListenerPosition();
    }

    static Vec3 getListenerVelocity()
    {
        return getSoundManager().getListenerVelocity();
    }

    static Vec3 getListenerForward()
    {
        return getSoundManager().getListenerForward();
    }

    static Vec3 getListenerUp()
    {
        return getSoundManager().getListenerUp();
    }

    static const std::string getError()
    {
        return getSoundManager().getError();
    }

    static void clearError()
    {
        getSoundManager().clearError();
    }

    static bool isHandleValid(size_t handle)
    {
        return getSoundManager().isHandleValid(handle);
    }
} // namespace soundcoe