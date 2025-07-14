#include <soundcoe.hpp>

namespace soundcoe
{
    static SoundManager s_soundManager;

    bool initialize(const std::string &audioRootDirectory, size_t maxSources, size_t maxCacheSizeMB, LogLevel level)
    {
        return s_soundManager.initialize(audioRootDirectory, maxSources, maxCacheSizeMB, level);
    }

    void shutdown()
    {
        s_soundManager.shutdown();
    }

    bool isInitialized()
    {
        return s_soundManager.isInitialized();
    }

    bool preloadScene(const std::string &sceneName)
    {
        return s_soundManager.preloadScene(sceneName);
    }

    bool unloadScene(const std::string &sceneName)
    {
        return s_soundManager.unloadScene(sceneName);
    }

    bool isSceneLoaded(const std::string &sceneName)
    {
        return s_soundManager.isSceneLoaded(sceneName);
    }

    void update()
    {
        s_soundManager.update();
    }

    SoundHandle playSound(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority)
    {
        return s_soundManager.playSound(filename, volume, pitch, loop, priority);
    }

    SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity, float volume, float pitch, bool loop, SoundPriority priority)
    {
        return s_soundManager.playSound3D(filename, position, velocity, volume, pitch, loop, priority);
    }

    MusicHandle playMusic(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority)
    {
        return s_soundManager.playMusic(filename, volume, pitch, loop, priority);
    }

    bool pauseSound(SoundHandle handle)
    {
        return s_soundManager.pauseSound(handle);
    }

    bool pauseMusic(MusicHandle handle)
    {
        return s_soundManager.pauseMusic(handle);
    }

    bool pauseAllSounds()
    {
        return s_soundManager.pauseAllSounds();
    }

    bool pauseAllMusic()
    {
        return s_soundManager.pauseAllMusic();
    }

    bool pauseAll()
    {
        return s_soundManager.pauseAll();
    }

    bool resumeSound(SoundHandle handle)
    {
        return s_soundManager.resumeSound(handle);
    }

    bool resumeMusic(MusicHandle handle)
    {
        return s_soundManager.resumeMusic(handle);
    }

    bool resumeAllSounds()
    {
        return s_soundManager.resumeAllSounds();
    }

    bool resumeAllMusic()
    {
        return s_soundManager.resumeAllMusic();
    }

    bool resumeAll()
    {
        return s_soundManager.resumeAll();
    }

    bool stopSound(SoundHandle handle)
    {
        return s_soundManager.stopSound(handle);
    }

    bool stopMusic(MusicHandle handle)
    {
        return s_soundManager.stopMusic(handle);
    }

    bool stopAllSounds()
    {
        return s_soundManager.stopAllSounds();
    }

    bool stopAllMusic()
    {
        return s_soundManager.stopAllMusic();
    }

    bool stopAll()
    {
        return s_soundManager.stopAll();
    }

    bool setSoundVolume(SoundHandle handle, float volume)
    {
        return s_soundManager.setSoundVolume(handle, volume);
    }

    bool setMusicVolume(MusicHandle handle, float volume)
    {
        return s_soundManager.setMusicVolume(handle, volume);
    }

    bool setSoundPitch(SoundHandle handle, float pitch)
    {
        return s_soundManager.setSoundPitch(handle, pitch);
    }

    bool setMusicPitch(MusicHandle handle, float pitch)
    {
        return s_soundManager.setMusicPitch(handle, pitch);
    }

    bool setSoundPosition(SoundHandle handle, const Vec3 &position)
    {
        return s_soundManager.setSoundPosition(handle, position);
    }

    bool setSoundVelocity(SoundHandle handle, const Vec3 &velocity)
    {
        return s_soundManager.setSoundVelocity(handle, velocity);
    }

    bool isSoundPlaying(SoundHandle handle)
    {
        return s_soundManager.isSoundPlaying(handle);
    }

    bool isMusicPlaying(MusicHandle handle)
    {
        return s_soundManager.isMusicPlaying(handle);
    }

    bool isSoundPaused(SoundHandle handle)
    {
        return s_soundManager.isSoundPaused(handle);
    }

    bool isMusicPaused(MusicHandle handle)
    {
        return s_soundManager.isMusicPaused(handle);
    }

    bool isSoundStopped(SoundHandle handle)
    {
        return s_soundManager.isSoundStopped(handle);
    }

    bool isMusicStopped(MusicHandle handle)
    {
        return s_soundManager.isMusicStopped(handle);
    }

    size_t getActiveSoundsCount()
    {
        return s_soundManager.getActiveSoundsCount();
    }

    size_t getActiveMusicCount()
    {
        return s_soundManager.getActiveMusicCount();
    }

    SoundHandle fadeInSound(const std::string &filename, float duration, float volume, float pitch, bool loop, SoundPriority priority)
    {
        return s_soundManager.fadeInSound(filename, duration, volume, pitch, loop, priority);
    }

    MusicHandle fadeInMusic(const std::string &filename, float duration, float volume, float pitch, bool loop, SoundPriority priority)
    {
        return s_soundManager.fadeInMusic(filename, duration, volume, pitch, loop, priority);
    }

    bool fadeOutSound(SoundHandle handle, float duration)
    {
        return s_soundManager.fadeOutSound(handle, duration);
    }

    bool fadeOutMusic(MusicHandle handle, float duration)
    {
        return s_soundManager.fadeOutMusic(handle, duration);
    }

    bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration)
    {
        return s_soundManager.fadeToVolumeSound(handle, targetVolume, duration);
    }

    bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration)
    {
        return s_soundManager.fadeToVolumeMusic(handle, targetVolume, duration);
    }

    bool setMasterVolume(float volume)
    {
        return s_soundManager.setMasterVolume(volume);
    }

    bool setMasterSoundsVolume(float volume)
    {
        return s_soundManager.setMasterSoundsVolume(volume);
    }

    bool setMasterMusicVolume(float volume)
    {
        return s_soundManager.setMasterMusicVolume(volume);
    }

    bool setMasterPitch(float pitch)
    {
        return s_soundManager.setMasterPitch(pitch);
    }

    bool setMasterSoundsPitch(float pitch)
    {
        return s_soundManager.setMasterSoundsPitch(pitch);
    }

    bool setMasterMusicPitch(float pitch)
    {
        return s_soundManager.setMasterMusicPitch(pitch);
    }

    float getMasterVolume()
    {
        return s_soundManager.getMasterVolume();
    }

    float getMasterSoundsVolume()
    {
        return s_soundManager.getMasterSoundsVolume();
    }

    float getMasterMusicVolume()
    {
        return s_soundManager.getMasterMusicVolume();
    }

    float getMasterPitch()
    {
        return s_soundManager.getMasterPitch();
    }

    float getMasterSoundsPitch()
    {
        return s_soundManager.getMasterSoundsPitch();
    }

    float getMasterMusicPitch()
    {
        return s_soundManager.getMasterMusicPitch();
    }

    bool muteAllSounds()
    {
        return s_soundManager.muteAllSounds();
    }

    bool muteAllMusic()
    {
        return s_soundManager.muteAllMusic();
    }

    bool muteAll()
    {
        return s_soundManager.muteAll();
    }

    bool unmuteAllSounds()
    {
        return s_soundManager.unmuteAllSounds();
    }

    bool unmuteAllMusic()
    {
        return s_soundManager.unmuteAllMusic();
    }

    bool unmuteAll()
    {
        return s_soundManager.unmuteAll();
    }

    bool isMuted()
    {
        return s_soundManager.isMuted();
    }

    bool isSoundsMuted()
    {
        return s_soundManager.isSoundsMuted();
    }

    bool isMusicMuted()
    {
        return s_soundManager.isMusicMuted();
    }

    bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up)
    {
        return s_soundManager.updateListener(position, velocity, forward, up);
    }

    bool setListenerPosition(const Vec3 &position)
    {
        return s_soundManager.setListenerPosition(position);
    }

    bool setListenerVelocity(const Vec3 &velocity)
    {
        return s_soundManager.setListenerVelocity(velocity);
    }

    bool setListenerForward(const Vec3 &forward)
    {
        return s_soundManager.setListenerForward(forward);
    }

    bool setListenerUp(const Vec3 &up)
    {
        return s_soundManager.setListenerUp(up);
    }

    Vec3 getListenerPosition()
    {
        return s_soundManager.getListenerPosition();
    }

    Vec3 getListenerVelocity()
    {
        return s_soundManager.getListenerVelocity();
    }

    Vec3 getListenerForward()
    {
        return s_soundManager.getListenerForward();
    }

    Vec3 getListenerUp()
    {
        return s_soundManager.getListenerUp();
    }

    const std::string getError()
    {
        return s_soundManager.getError();
    }

    void clearError()
    {
        s_soundManager.clearError();
    }

    bool isHandleValid(size_t handle)
    {
        return s_soundManager.isHandleValid(handle);
    }
} // namespace soundcoe
