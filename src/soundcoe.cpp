#include <soundcoe.hpp>
#include <soundcoe/playback/sound_manager.hpp>
#include <logcoe.hpp>
#include <cassert>

namespace soundcoe
{
    namespace detail
    {
        SoundManager& getSoundManagerInstance()
        {
            static SoundManager s_soundManager;
            return s_soundManager;
        }
    }

    bool initialize(const std::string &audioRootDirectory, size_t maxSources,
                    size_t maxCacheSizeMB, const std::string &soundSubdir,
                    const std::string &musicSubdir, LogLevel level)
    {
        return detail::getSoundManagerInstance().initialize(audioRootDirectory, maxSources, maxCacheSizeMB,
                                                            soundSubdir, musicSubdir, level);
    }

    void shutdown()
    {
        detail::getSoundManagerInstance().shutdown();
    }

    bool isInitialized()
    {
        return detail::getSoundManagerInstance().isInitialized();
    }

    bool preloadScene(const std::string &sceneName)
    {
        return detail::getSoundManagerInstance().preloadScene(sceneName);
    }

    bool unloadScene(const std::string &sceneName)
    {
        return detail::getSoundManagerInstance().unloadScene(sceneName);
    }

    bool isSceneLoaded(const std::string &sceneName)
    {
        return detail::getSoundManagerInstance().isSceneLoaded(sceneName);
    }

    void update()
    {
        detail::getSoundManagerInstance().update();
    }

    SoundHandle playSound(const std::string &filename, float volume, float pitch, bool loop,
                          SoundPriority priority)
    {
        return detail::getSoundManagerInstance().playSound(filename, volume, pitch, loop, priority);
    }

    SoundHandle playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity,
                            float volume, float pitch, bool loop,
                            SoundPriority priority)
    {
        return detail::getSoundManagerInstance().playSound3D(filename, position, velocity, volume, pitch, loop, priority);
    }

    MusicHandle playMusic(const std::string &filename, float volume, float pitch, bool loop,
                          SoundPriority priority)
    {
        return detail::getSoundManagerInstance().playMusic(filename, volume, pitch, loop, priority);
    }

    bool pauseSound(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().pauseSound(handle);
    }

    bool pauseMusic(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().pauseMusic(handle);
    }

    void pauseAllSounds()
    {
        bool succeed = detail::getSoundManagerInstance().pauseAllSounds();
        assert(succeed);
    }

    void pauseAllMusic()
    {
        bool succeed = detail::getSoundManagerInstance().pauseAllMusic();
        assert(succeed);
    }

    void pauseAll()
    {
        bool succeed = detail::getSoundManagerInstance().pauseAll();
        assert(succeed);
    }

    bool resumeSound(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().resumeSound(handle);
    }

    bool resumeMusic(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().resumeMusic(handle);
    }

    void resumeAllSounds()
    {
        bool succeed = detail::getSoundManagerInstance().resumeAllSounds();
        assert(succeed);
    }

    void resumeAllMusic()
    {
        bool succeed = detail::getSoundManagerInstance().resumeAllMusic();
        assert(succeed);
    }

    void resumeAll()
    {
        bool succeed = detail::getSoundManagerInstance().resumeAll();
        assert(succeed);
    }

    bool stopSound(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().stopSound(handle);
    }

    bool stopMusic(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().stopMusic(handle);
    }

    void stopAllSounds()
    {
        bool succeed = detail::getSoundManagerInstance().stopAllSounds();
        assert(succeed);
    }

    void stopAllMusic()
    {
        bool succeed = detail::getSoundManagerInstance().stopAllMusic();
        assert(succeed);
    }

    void stopAll()
    {
        bool succeed = detail::getSoundManagerInstance().stopAll();
        assert(succeed);
    }

    bool setSoundVolume(SoundHandle handle, float volume)
    {
        return detail::getSoundManagerInstance().setSoundVolume(handle, volume);
    }

    bool setMusicVolume(MusicHandle handle, float volume)
    {
        return detail::getSoundManagerInstance().setMusicVolume(handle, volume);
    }

    bool setSoundPitch(SoundHandle handle, float pitch)
    {
        return detail::getSoundManagerInstance().setSoundPitch(handle, pitch);
    }

    bool setMusicPitch(MusicHandle handle, float pitch)
    {
        return detail::getSoundManagerInstance().setMusicPitch(handle, pitch);
    }

    bool setSoundPosition(SoundHandle handle, const Vec3 &position)
    {
        return detail::getSoundManagerInstance().setSoundPosition(handle, position);
    }

    bool setSoundVelocity(SoundHandle handle, const Vec3 &velocity)
    {
        return detail::getSoundManagerInstance().setSoundVelocity(handle, velocity);
    }

    bool isSoundPlaying(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().isSoundPlaying(handle);
    }

    bool isMusicPlaying(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().isMusicPlaying(handle);
    }

    bool isSoundPaused(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().isSoundPaused(handle);
    }

    bool isMusicPaused(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().isMusicPaused(handle);
    }

    bool isSoundStopped(SoundHandle handle)
    {
        return detail::getSoundManagerInstance().isSoundStopped(handle);
    }

    bool isMusicStopped(MusicHandle handle)
    {
        return detail::getSoundManagerInstance().isMusicStopped(handle);
    }

    size_t getActiveSoundsCount()
    {
        return detail::getSoundManagerInstance().getActiveSoundsCount();
    }

    size_t getActiveMusicCount()
    {
        return detail::getSoundManagerInstance().getActiveMusicCount();
    }

    SoundHandle fadeInSound(const std::string &filename, float duration,
                            float volume, float pitch, bool loop,
                            SoundPriority priority)
    {
        return detail::getSoundManagerInstance().fadeInSound(filename, duration, volume, pitch, loop, priority);
    }

    MusicHandle fadeInMusic(const std::string &filename, float duration,
                            float volume, float pitch, bool loop,
                            SoundPriority priority)
    {
        return detail::getSoundManagerInstance().fadeInMusic(filename, duration, volume, pitch, loop, priority);
    }

    bool fadeOutSound(SoundHandle handle, float duration)
    {
        return detail::getSoundManagerInstance().fadeOutSound(handle, duration);
    }

    bool fadeOutMusic(MusicHandle handle, float duration)
    {
        return detail::getSoundManagerInstance().fadeOutMusic(handle, duration);
    }

    bool fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration)
    {
        return detail::getSoundManagerInstance().fadeToVolumeSound(handle, targetVolume, duration);
    }

    bool fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration)
    {
        return detail::getSoundManagerInstance().fadeToVolumeMusic(handle, targetVolume, duration);
    }

    bool setMasterVolume(float volume)
    {
        return detail::getSoundManagerInstance().setMasterVolume(volume);
    }

    bool setMasterSoundsVolume(float volume)
    {
        return detail::getSoundManagerInstance().setMasterSoundsVolume(volume);
    }

    bool setMasterMusicVolume(float volume)
    {
        return detail::getSoundManagerInstance().setMasterMusicVolume(volume);
    }

    bool setMasterPitch(float pitch)
    {
        return detail::getSoundManagerInstance().setMasterPitch(pitch);
    }

    bool setMasterSoundsPitch(float pitch)
    {
        return detail::getSoundManagerInstance().setMasterSoundsPitch(pitch);
    }

    bool setMasterMusicPitch(float pitch)
    {
        return detail::getSoundManagerInstance().setMasterMusicPitch(pitch);
    }

    float getMasterVolume()
    {
        return detail::getSoundManagerInstance().getMasterVolume();
    }

    float getMasterSoundsVolume()
    {
        return detail::getSoundManagerInstance().getMasterSoundsVolume();
    }

    float getMasterMusicVolume()
    {
        return detail::getSoundManagerInstance().getMasterMusicVolume();
    }

    float getMasterPitch()
    {
        return detail::getSoundManagerInstance().getMasterPitch();
    }

    float getMasterSoundsPitch()
    {
        return detail::getSoundManagerInstance().getMasterSoundsPitch();
    }

    float getMasterMusicPitch()
    {
        return detail::getSoundManagerInstance().getMasterMusicPitch();
    }

    void muteAllSounds()
    {
        bool succeed = detail::getSoundManagerInstance().muteAllSounds();
        assert(succeed);
    }

    void muteAllMusic()
    {
        bool succeed = detail::getSoundManagerInstance().muteAllMusic();
        assert(succeed);
    }

    void muteAll()
    {
        bool succeed = detail::getSoundManagerInstance().muteAll();
        assert(succeed);
    }

    void unmuteAllSounds()
    {
        bool succeed = detail::getSoundManagerInstance().unmuteAllSounds();
        assert(succeed);
    }

    void unmuteAllMusic()
    {
        bool succeed = detail::getSoundManagerInstance().unmuteAllMusic();
        assert(succeed);
    }

    void unmuteAll()
    {
        bool succeed = detail::getSoundManagerInstance().unmuteAll();
        assert(succeed);
    }

    bool isMuted()
    {
        return detail::getSoundManagerInstance().isMuted();
    }

    bool isSoundsMuted()
    {
        return detail::getSoundManagerInstance().isSoundsMuted();
    }

    bool isMusicMuted()
    {
        return detail::getSoundManagerInstance().isMusicMuted();
    }

    bool updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up)
    {
        return detail::getSoundManagerInstance().updateListener(position, velocity, forward, up);
    }

    bool setListenerPosition(const Vec3 &position)
    {
        return detail::getSoundManagerInstance().setListenerPosition(position);
    }

    bool setListenerVelocity(const Vec3 &velocity)
    {
        return detail::getSoundManagerInstance().setListenerVelocity(velocity);
    }

    bool setListenerForward(const Vec3 &forward)
    {
        return detail::getSoundManagerInstance().setListenerForward(forward);
    }

    bool setListenerUp(const Vec3 &up)
    {
        return detail::getSoundManagerInstance().setListenerUp(up);
    }

    Vec3 getListenerPosition()
    {
        return detail::getSoundManagerInstance().getListenerPosition();
    }

    Vec3 getListenerVelocity()
    {
        return detail::getSoundManagerInstance().getListenerVelocity();
    }

    Vec3 getListenerForward()
    {
        return detail::getSoundManagerInstance().getListenerForward();
    }

    Vec3 getListenerUp()
    {
        return detail::getSoundManagerInstance().getListenerUp();
    }

    const std::string getError()
    {
        return detail::getSoundManagerInstance().getError();
    }

    void clearError()
    {
        detail::getSoundManagerInstance().clearError();
    }

    bool isHandleValid(size_t handle)
    {
        return detail::getSoundManagerInstance().isHandleValid(handle);
    }
} // namespace soundcoe