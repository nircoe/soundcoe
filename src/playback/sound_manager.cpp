#include <soundcoe/playback/sound_manager.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <AL/al.h>
#include <logcoe.hpp>
#include <functional>

namespace soundcoe
{
    void SoundManager::updateAllSoundsVolume()
    {
        updateAllAudioProperty(m_activeSounds,
                               [](std::unique_ptr<SoundSource> &audio, float value) { audio->setVolume(value); },
                               [](const ActiveAudio &audio) { return audio.m_baseVolume; },
                               m_masterVolume, m_masterSoundsVolume);
    }

    void SoundManager::updateAllMusicVolume()
    {
        updateAllAudioProperty(m_activeMusic,
                               [](std::unique_ptr<SoundSource> &audio, float value) { audio->setVolume(value); },
                               [](const ActiveAudio &audio) { return audio.m_baseVolume; },
                               m_masterVolume, m_masterMusicVolume);
    }

    void SoundManager::updateAllVolume()
    {
        updateAllSoundsVolume();
        updateAllMusicVolume();
    }

    void SoundManager::updateAllSoundsPitch()
    {
        updateAllAudioProperty(m_activeSounds,
                               [](std::unique_ptr<SoundSource> &audio, float value) { audio->setPitch(value); },
                               [](const ActiveAudio &audio) { return audio.m_basePitch; },
                               m_masterPitch, m_masterSoundsPitch);
    }

    void SoundManager::updateAllMusicPitch()
    {
        updateAllAudioProperty(m_activeMusic,
                               [](std::unique_ptr<SoundSource> &audio, float value) { audio->setPitch(value); },
                               [](const ActiveAudio &audio) { return audio.m_basePitch; },
                               m_masterPitch, m_masterMusicPitch);
    }

    void SoundManager::updateAllPitch()
    {
        updateAllSoundsPitch();
        updateAllMusicPitch();
    }

    bool SoundManager::setListenerPositionImpl(const Vec3 &position)
    {
        ALfloat pos[3] = {position.x, position.y, position.z};
        alListenerfv(AL_POSITION, pos);
        if (ErrorHandler::checkOpenALError("Set Listener Position"))
            return false;
        m_listenerPosition = position;
        return true;
    }

    bool SoundManager::setListenerVelocityImpl(const Vec3 &velocity)
    {
        ALfloat vel[3] = {velocity.x, velocity.y, velocity.z};
        alListenerfv(AL_VELOCITY, vel);
        if (ErrorHandler::checkOpenALError("Set Listener Velocity"))
            return false;
        m_listenerVelocity = velocity;
        return true;
    }

    bool SoundManager::setListenerOrientationImpl(const Vec3 &forward, const Vec3 &up)
    {
        ALfloat orientation[6] = {forward.x, forward.y, forward.z,
                                  up.x, up.y, up.z};
        alListenerfv(AL_ORIENTATION, orientation);
        if (ErrorHandler::checkOpenALError("Set Listener Forward and Up Vectors"))
            return false;
        m_listenerForward = forward;
        m_listenerUp = up;
        return true;
    }

    SoundManager::SoundManager() {}

    SoundManager::SoundManager(const std::string &audioRootDirectory)
    {
        alListenerf(AL_GAIN, 1.0f);

        // to be continued
    }

    SoundManager::~SoundManager() {}

    void SoundManager::initialize(const std::string &audioRootDirectory)
    {
        if (m_initialized)
        {
            logcoe::warning("Need to shutdown SoundManager before initialize it again");
            return;
        }
        alListenerf(AL_GAIN, 1.0f);

        // to be continued
    }

    void SoundManager::shutdown() {}

    bool SoundManager::isInitialized() const { return false; }

    bool SoundManager::preloadScene(const std::string &sceneName) { return false; }

    bool SoundManager::unloadScene(const std::string &sceneName) { return false; }

    bool SoundManager::isSceneLoaded(const std::string &sceneName) { return false; }

    void SoundManager::update() {}

    SoundHandle SoundManager::playSound(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority) { return INVALID_SOUND_HANDLE; }

    SoundHandle SoundManager::playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity, float volume, float pitch, bool loop, SoundPriority priority) { return INVALID_SOUND_HANDLE; }

    MusicHandle SoundManager::playMusic(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority) { return INVALID_MUSIC_HANDLE; }

    MusicHandle SoundManager::streamMusic(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority) { return INVALID_MUSIC_HANDLE; }

    bool SoundManager::pauseSound(SoundHandle handle) { return false; }

    bool SoundManager::pauseMusic(MusicHandle handle) { return false; }

    bool SoundManager::pauseAllSounds() { return false; }

    bool SoundManager::pauseAllMusic() { return false; }

    bool SoundManager::pauseAll() { return false; }

    bool SoundManager::resumeSound(SoundHandle handle) { return false; }

    bool SoundManager::resumeMusic(MusicHandle handle) { return false; }

    bool SoundManager::resumeAllSounds() { return false; }

    bool SoundManager::resumeAllMusic() { return false; }

    bool SoundManager::resumeAll() { return false; }

    bool SoundManager::stopSound(SoundHandle handle) { return false; }

    bool SoundManager::stopMusic(MusicHandle handle) { return false; }

    bool SoundManager::stopAllSounds() { return false; }

    bool SoundManager::stopAllMusic() { return false; }

    bool SoundManager::stopAll() { return false; }

    bool SoundManager::setSoundVolume(SoundHandle handle, float volume) { return false; }

    bool SoundManager::setMusicVolume(MusicHandle handle, float volume) { return false; }

    bool SoundManager::setSoundPitch(SoundHandle handle, float pitch) { return false; }

    bool SoundManager::setMusicPitch(MusicHandle handle, float pitch) { return false; }

    bool SoundManager::setSoundPosition(SoundHandle handle, const Vec3 &position) { return false; }

    bool SoundManager::setSoundVelocity(SoundHandle handle, const Vec3 &velocity) { return false; }

    bool SoundManager::isSoundPlaying(SoundHandle handle) const { return false; }

    bool SoundManager::isMusicPlaying(MusicHandle handle) const { return false; }

    bool SoundManager::isSoundPaused(SoundHandle handle) const { return false; }

    bool SoundManager::isMusicPaused(MusicHandle handle) const { return false; }

    bool SoundManager::isSoundStopped(SoundHandle handle) const { return false; }

    bool SoundManager::isMusicStopped(MusicHandle handle) const { return false; }

    size_t SoundManager::getActiveSoundCount() const { return 0; }

    size_t SoundManager::getActiveMusicCount() const { return 0; }

    bool SoundManager::fadeInSound(SoundHandle handle, float duration) { return false; }

    bool SoundManager::fadeInMusic(MusicHandle handle, float duration) { return false; }

    bool SoundManager::fadeOutSound(SoundHandle handle, float duration) { return false; }

    bool SoundManager::fadeOutMusic(MusicHandle handle, float duration) { return false; }

    bool SoundManager::fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration) { return false; }

    bool SoundManager::fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration) { return false; }

    bool SoundManager::setMasterVolume(float volume)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterVolume = volume;
        updateAllVolume();
    }

    bool SoundManager::setMasterSoundsVolume(float volume)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterSoundsVolume = volume;
        updateAllSoundsVolume();
    }

    bool SoundManager::setMasterMusicVolume(float volume)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterMusicVolume = volume;
        updateAllMusicVolume();
    }

    bool SoundManager::setMasterPitch(float pitch)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterPitch = pitch;
        updateAllPitch();
    }

    bool SoundManager::setMasterSoundsPitch(float pitch)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterSoundsPitch = pitch;
        updateAllSoundsPitch();
    }

    bool SoundManager::setMasterMusicPitch(float pitch)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterMusicPitch = pitch;
        updateAllMusicPitch();
    }

    float SoundManager::getMasterVolume() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_masterVolume;
    }

    float SoundManager::getMasterSoundsVolume() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_masterSoundsVolume;
    }

    float SoundManager::getMasterMusicVolume() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_masterMusicVolume;
    }

    float SoundManager::getMasterPitch() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_masterPitch;
    }

    float SoundManager::getMasterSoundsPitch() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_masterSoundsPitch;
    }

    float SoundManager::getMasterMusicPitch() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_masterMusicPitch;
    }

    bool SoundManager::muteAllSounds()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_soundsMute = true;
        return true;
    }

    bool SoundManager::muteAllMusic()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_musicMute = true;
        return true;
    }

    bool SoundManager::muteAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_mute = true;
        return true;
    }

    bool SoundManager::isMuted() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_mute;
    }

    bool SoundManager::isSoundsMuted() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_soundsMute;
    }

    bool SoundManager::isMusicMuted() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_musicMute;
    }

    bool SoundManager::updateListener(const Vec3 &position, const Vec3 &velocity, const Vec3 &forward, const Vec3 &up)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setListenerPositionImpl(position) &&
               setListenerVelocityImpl(velocity) &&
               setListenerOrientationImpl(forward, up);
    }

    bool SoundManager::setListenerPosition(const Vec3 &position)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setListenerPositionImpl(position);
    }

    bool SoundManager::setListenerVelocity(const Vec3 &velocity)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setListenerVelocityImpl(velocity);
    }

    bool SoundManager::setListenerForward(const Vec3 &forward)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setListenerOrientationImpl(forward, m_listenerUp);
    }

    bool SoundManager::setListenerUp(const Vec3 &up)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setListenerOrientationImpl(m_listenerForward, up);
    }

    Vec3 SoundManager::getListenerPosition()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_listenerPosition;
    }

    Vec3 SoundManager::getListenerVelocity()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_listenerVelocity;
    }

    Vec3 SoundManager::getListenerForward()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_listenerForward;
    }

    Vec3 SoundManager::getListenerUp()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_listenerUp;
    }

    const std::string SoundManager::getError()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string error(m_lastError);
        m_lastError = "";
        return error;
    }

    void SoundManager::clearError()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lastError = "";
    }

    bool SoundManager::isHandleValid(SoundHandle handle) { return handle != INVALID_SOUND_HANDLE; }

    bool SoundManager::isHandleValid(MusicHandle handle) { return handle != INVALID_MUSIC_HANDLE; }

} // namespace soundcoe