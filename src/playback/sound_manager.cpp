#include <soundcoe/playback/sound_manager.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <AL/al.h>
#include <logcoe.hpp>
#include <functional>
#include <filesystem>

namespace soundcoe
{
    void SoundManager::updateAllSoundsVolume()
    {
        updateAllAudioProperty(m_activeSounds, 
                               [](std::unique_ptr<SoundSource> &audio, float value) { audio->setVolume(value); }, 
                               [&](const ActiveAudio &audio) { return (m_soundsMute || m_mute) ? 0.0f : audio.m_baseVolume; }, 
                               m_masterVolume, m_masterSoundsVolume);
    }

    void SoundManager::updateAllMusicVolume()
    {
        updateAllAudioProperty(m_activeMusic, 
                               [](std::unique_ptr<SoundSource> &audio, float value) { audio->setVolume(value); }, 
                               [&](const ActiveAudio &audio) { return (m_musicMute || m_mute) ? 0.0f : audio.m_baseVolume; }, 
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
                               [](const ActiveAudio &audio) { return audio.m_basePitch; }, m_masterPitch, m_masterMusicPitch);
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

    bool SoundManager::setError(const std::string &error)
    {
        m_hasError = true;
        m_lastError = error;
        logcoe::error(error);
        return false;
    }

    bool SoundManager::fadeToVolume(std::unordered_map<size_t, ActiveAudio> &activeAudio, size_t handle,
                                    float targetVolume, float duration, const std::string &method)
    {
        auto it = activeAudio.find(handle);
        if (it == activeAudio.end())
            return setError(method + ": Invalid handle");

        if (duration <= 0.0f)
            return setError(method + ": Fade duration must be positive");

        if (targetVolume < 0.0f)
            return setError(method + ": Fade target volume must be non-negative");

        ActiveAudio &audio = it->second;

        auto sourceAllocation = m_resourceManager.getSourceAllocation(audio.m_sourceIndex);
        if (!(sourceAllocation.has_value()) || !(sourceAllocation.value().m_active))
        {
            activeAudio.erase(it);
            return setError(method + ": Audio source is no longer active");
        }

        auto &source = sourceAllocation.value().m_source;
        if (!(source->isPlaying()))
            return setError(method + ": Cannot fadeToVolume audio that is not playing.");

        audio.m_isFading = true;
        audio.m_fadeStartVolume = audio.m_baseVolume;
        audio.m_fadeTargetVolume = targetVolume;
        audio.m_fadeDuration = duration;
        audio.m_fadeElapsed = 0.0f;
        return true;
    }

    bool SoundManager::fade(std::unordered_map<size_t, ActiveAudio> &activeAudio, size_t handle,
                            bool fadeIn, float duration, const std::string &method)
    {
        auto it = activeAudio.find(handle);
        if (it == activeAudio.end())
            return setError(method + ": Invalid handle");

        if (duration <= 0.0f)
            return setError(method + ": Fade duration must be positive");

        ActiveAudio &audio = it->second;

        auto sourceAllocation = m_resourceManager.getSourceAllocation(audio.m_sourceIndex);
        if (!(sourceAllocation.has_value()) || !(sourceAllocation.value().m_active))
        {
            activeAudio.erase(it);
            return setError(method + ": Audio source is no longer active");
        }

        auto &source = sourceAllocation.value().m_source;
        if (!fadeIn && !(source->isPlaying()))
            return setError(method + ": Cannot fade out audio that is not playing.");

        audio.m_isFading = true;
        audio.m_fadeStartVolume = fadeIn ? 0.0f : audio.m_baseVolume;
        audio.m_fadeTargetVolume = fadeIn ? audio.m_baseVolume : 0.0f;
        audio.m_fadeDuration = duration;
        audio.m_fadeElapsed = 0.0f;
        return true;
    }

    bool SoundManager::checkAudioState(std::unordered_map<size_t, ActiveAudio> &activeAudio, size_t handle, 
                                       SoundState state, const std::string &method)
    {
        m_lastError = "";
        m_hasError = false;

        auto it = activeAudio.find(handle);
        if (it == activeAudio.end())
            return setError(method + ": Invalid handle");

        ActiveAudio &audio = it->second;
        auto sourceAllocation = m_resourceManager.getSourceAllocation(audio.m_sourceIndex);
        if (!(sourceAllocation.has_value()) || !(sourceAllocation.value().m_active))
        {
            activeAudio.erase(it);
            return setError(method + ": Audio source is no longer active");
        }

        auto &source = sourceAllocation.value().m_source;
        if(state == SoundState::Playing) return source->isPlaying();
        if(state == SoundState::Paused)  return source->isPaused();
        if(state == SoundState::Stopped) return source->isStopped();
        
        return setError(method + ": Internal error - Invalid operation type");
    }

    bool SoundManager::audioOperation(std::unordered_map<size_t, ActiveAudio> &activeAudio, size_t handle,
                                      SoundState operation, const std::string &method)
    {
        auto it = activeAudio.find(handle);
        if (it == activeAudio.end())
            return setError(method + ": Invalid handle");

        ActiveAudio &audio = it->second;
        auto sourceAllocation = m_resourceManager.getSourceAllocation(audio.m_sourceIndex);
        if (!(sourceAllocation.has_value()) || !(sourceAllocation.value().m_active))
        {
            activeAudio.erase(it);
            return setError(method + ": Audio source is no longer active");
        }

        auto &source = sourceAllocation.value().m_source;
        if(operation == SoundState::Playing) return source->play();
        if(operation == SoundState::Paused)  return source->pause();
        if(operation == SoundState::Stopped) return source->stop();

        return setError(method + ": Internal error - Invalid operation type");
    }

    bool SoundManager::audioOperationAll(std::unordered_map<size_t, ActiveAudio> &activeAudio, SoundState operation, 
                                         const std::string &method)
    {
        for (auto it = activeAudio.begin(); it != activeAudio.end();)
        {
            ActiveAudio &audio = it->second;
            auto sourceAllocation = m_resourceManager.getSourceAllocation(audio.m_sourceIndex);
            if (!(sourceAllocation.has_value()) || !(sourceAllocation.value().m_active))
            {
                logcoe::warning(method + ": handle " + std::to_string(it->first) + " is no longer active");
                it = activeAudio.erase(it);
                continue;
            }

            auto &source = sourceAllocation.value().m_source;
            bool success = false;
            if(operation == SoundState::Playing)      success = source->play();
            else if(operation == SoundState::Paused)  success = source->pause();
            else if(operation == SoundState::Stopped) success = source->stop();
            else
                return setError(method + ": Internal error - Invalid operation type");

            if(!success)
                logcoe::warning(method + ": Failed to operate on handle - " + std::to_string(it->first));

            ++it;
        }

        return true;
    }

    size_t SoundManager::play(std::unordered_map<size_t, ActiveAudio> &activeAudio, const std::string &filename, 
                              float volume, float pitch, bool loop, SoundPriority priority, 
                              std::atomic<size_t> &nextHandle, const std::string &method,
                              bool is3D, const Vec3 &position, const Vec3 &velocity)
    {
        auto buffer = m_resourceManager.getBuffer(filename);
        if (!(buffer.has_value()))
        {
            logcoe::error(method + ": Failed to load the sound file");
            return INVALID_SOUND_HANDLE;
        }

        size_t poolIndex;
        auto source = m_resourceManager.acquireSource(poolIndex, priority);
        if (!(source.has_value()))
        {
            logcoe::error(method + ": Failed to acquire source");
            m_resourceManager.releaseBuffer(buffer.value());
            return INVALID_SOUND_HANDLE;
        }

        try
        {
            source->get().attachBuffer(buffer->get());
        }
        catch (const std::exception &e)
        {
            logcoe::error(method + ": Failed to attach buffer: " + std::string(e.what()));
            m_resourceManager.releaseSource(source.value());
            m_resourceManager.releaseBuffer(buffer.value());
            return INVALID_SOUND_HANDLE;
        }

        if (!(source->get().setVolume(volume)))
            logcoe::warning(method + ": Failed to set volume for " + filename);
        if (!(source->get().setPitch(pitch)))
            logcoe::warning(method + ": Failed to set pitch for " + filename);
        if (!(source->get().setLooping(loop)))
            logcoe::warning(method + ": Failed to set looping for " + filename);
        if(is3D)
        {
            if(!(source->get().setPosition(position)))
                logcoe::warning(method + ": Failed to set position for " + filename);
            if(!(source->get().setVelocity(velocity)))
                logcoe::warning(method + ": Failed to set velocity for " + filename);
        }
        if (!(source->get().play()))
        {
            logcoe::error(method + ": Failed to play the sound " + filename);
            m_resourceManager.releaseSource(source.value());
            m_resourceManager.releaseBuffer(buffer.value());
            return INVALID_SOUND_HANDLE;
        }

        ActiveAudio audio;
        audio.m_sourceIndex = poolIndex;
        audio.m_filename = filename;
        audio.m_baseVolume = volume;
        audio.m_basePitch = pitch;
        audio.m_loop = loop;
        audio.m_stream = buffer->get().isStreaming();

        activeAudio[nextHandle] = std::move(audio);

        return nextHandle++;
    }

    SoundManager::SoundManager() : m_resourceManager(), m_nextSoundHandle(1), m_nextMusicHandle(1),
                                   m_activeSounds(), m_activeMusic(), m_listenerPosition(),
                                   m_listenerVelocity(), m_listenerForward(), m_listenerUp(),
                                   m_lastUpdate() { }

    SoundManager::~SoundManager() { shutdown(); }

    bool SoundManager::initialize(const std::string &audioRootDirectory, size_t maxSources, size_t maxCacheSizeMB, 
                                  LogLevel level)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_initialized)
        {
            logcoe::warning("Need to shutdown SoundManager before initialize it again");
            return false;
        }

        logcoe::initialize(level, "soundcoe");

        try { m_resourceManager.initialize(audioRootDirectory, maxSources, maxCacheSizeMB); }
        catch(const std::exception &e)
        {
            logcoe::error("Failed to create Resource Manager: " + std::string(e.what()));
            logcoe::shutdown();
            return false;
        }

        alListenerf(AL_GAIN, 1.0f);

        std::filesystem::path rootDirectory(audioRootDirectory);
        std::filesystem::path generalAudioDirectory(rootDirectory / "general");
        if(std::filesystem::exists(generalAudioDirectory) && std::filesystem::is_directory(generalAudioDirectory))
        {
            if(!preloadScene("general"))
            {
                logcoe::error("Failed to load general audio subdirectory");
                m_resourceManager.shutdown();
                logcoe::shutdown();
                return false;
            }
        }
        else
            logcoe::warning("There is no general audio subdirectory");

        m_initialized = true;
        return true;
    }

    void SoundManager::shutdown() 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_nextSoundHandle = 1;
        m_nextMusicHandle = 1;

        m_activeSounds.clear();
        m_activeMusic.clear();

        m_masterVolume          = 1.0f;
        m_masterSoundsVolume    = 1.0f;
        m_masterMusicVolume     = 1.0f;
        m_masterPitch           = 1.0f;
        m_masterSoundsPitch     = 1.0f;
        m_masterMusicPitch      = 1.0f;

        m_mute          = false;
        m_soundsMute    = false;
        m_musicMute     = false;

        m_listenerPosition  = Vec3::zero();
        m_listenerVelocity  = Vec3::zero();
        m_listenerForward   = Vec3::zero();
        m_listenerUp        = Vec3::zero();

        m_lastUpdate    = std::chrono::steady_clock::time_point();
        m_firstUpdate   = true;

        m_lastError = "";
        m_hasError  = false;

        m_resourceManager.shutdown();
        logcoe::shutdown();
        m_initialized = false;
    }

    bool SoundManager::isInitialized() const 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_initialized;
    }

    bool SoundManager::preloadScene(const std::string &sceneName) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_resourceManager.preloadDirectory(sceneName);
    }

    bool SoundManager::unloadScene(const std::string &sceneName)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_resourceManager.unloadDirectory(sceneName);
    }

    bool SoundManager::isSceneLoaded(const std::string &sceneName) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_resourceManager.isDirectoryLoaded(sceneName);
    }

    void SoundManager::update() {}

    SoundHandle SoundManager::playSound(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return play(m_activeSounds, filename, volume, pitch, loop, priority, m_nextSoundHandle, "playSound");
    }

    SoundHandle SoundManager::playSound3D(const std::string &filename, const Vec3 &position, const Vec3 &velocity, 
                                          float volume, float pitch, bool loop, SoundPriority priority) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return play(m_activeSounds, filename, volume, pitch, loop, priority, m_nextSoundHandle, "playSound3D",
                    true, position, velocity);
    }

    MusicHandle SoundManager::playMusic(const std::string &filename, float volume, float pitch, bool loop, SoundPriority priority)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return play(m_activeMusic, filename, volume, pitch, loop, priority, m_nextMusicHandle, "playMusic");
    }

    bool SoundManager::pauseSound(SoundHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperation(m_activeSounds, handle, SoundState::Paused, "pauseSound");
    }

    bool SoundManager::pauseMusic(MusicHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperation(m_activeMusic, handle, SoundState::Paused, "pauseMusic");
    }

    bool SoundManager::pauseAllSounds()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperationAll(m_activeSounds, SoundState::Paused, "pauseAllSounds");
    }

    bool SoundManager::pauseAllMusic()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperationAll(m_activeMusic, SoundState::Paused, "pauseAllMusic");
    }

    bool SoundManager::pauseAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperationAll(m_activeSounds, SoundState::Paused, "pauseAll") &&
               audioOperationAll(m_activeMusic, SoundState::Paused, "pauseAll");
    }

    bool SoundManager::resumeSound(SoundHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperation(m_activeSounds, handle, SoundState::Playing, "resumeSound");
    }

    bool SoundManager::resumeMusic(MusicHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperation(m_activeMusic, handle, SoundState::Playing, "resumeMusic");
    }

    bool SoundManager::resumeAllSounds()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperationAll(m_activeSounds, SoundState::Playing, "resumeAllSounds");
    }

    bool SoundManager::resumeAllMusic()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperationAll(m_activeMusic, SoundState::Playing, "resumeAllMusic");
    }

    bool SoundManager::resumeAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperationAll(m_activeSounds, SoundState::Playing, "resumeAll") &&
               audioOperationAll(m_activeMusic, SoundState::Playing, "resumeAll");
    }

    bool SoundManager::stopSound(SoundHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperation(m_activeSounds, handle, SoundState::Stopped, "stopSound");
    }

    bool SoundManager::stopMusic(MusicHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperation(m_activeMusic, handle, SoundState::Stopped, "stopMusic");
    }

    bool SoundManager::stopAllSounds()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperationAll(m_activeSounds, SoundState::Stopped, "stopAllSounds");
    }

    bool SoundManager::stopAllMusic()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperationAll(m_activeMusic, SoundState::Stopped, "stopAllMusic");
    }

    bool SoundManager::stopAll() 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return audioOperationAll(m_activeSounds, SoundState::Stopped, "stopAll") &&
               audioOperationAll(m_activeMusic, SoundState::Stopped, "stopAll");
    }

    bool SoundManager::setSoundVolume(SoundHandle handle, float volume)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setAudioProperty(m_activeSounds, handle, volume, PropertyType::Volume, "setSoundVolume");
    }

    bool SoundManager::setMusicVolume(MusicHandle handle, float volume)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setAudioProperty(m_activeMusic, handle, volume, PropertyType::Volume, "setMusicVolume");
    }

    bool SoundManager::setSoundPitch(SoundHandle handle, float pitch)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setAudioProperty(m_activeSounds, handle, pitch, PropertyType::Pitch, "setSoundPitch");
    }

    bool SoundManager::setMusicPitch(MusicHandle handle, float pitch)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setAudioProperty(m_activeMusic, handle, pitch, PropertyType::Pitch, "setMusicPitch");
    }

    bool SoundManager::setSoundPosition(SoundHandle handle, const Vec3 &position)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setAudioProperty(m_activeSounds, handle, position, PropertyType::Position, "setSoundPosition");
    }

    bool SoundManager::setSoundVelocity(SoundHandle handle, const Vec3 &velocity)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return setAudioProperty(m_activeSounds, handle, velocity, PropertyType::Velocity, "setSoundVelocity");
    }

    bool SoundManager::isSoundPlaying(SoundHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return checkAudioState(m_activeSounds, handle, SoundState::Playing, "isSoundPlaying");
    }

    bool SoundManager::isMusicPlaying(MusicHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return checkAudioState(m_activeMusic, handle, SoundState::Playing, "isMusicPlaying");
    }

    bool SoundManager::isSoundPaused(SoundHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return checkAudioState(m_activeSounds, handle, SoundState::Paused, "isSoundPaused");
    }

    bool SoundManager::isMusicPaused(MusicHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return checkAudioState(m_activeMusic, handle, SoundState::Paused, "isMusicPaused");
    }

    bool SoundManager::isSoundStopped(SoundHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return checkAudioState(m_activeSounds, handle, SoundState::Stopped, "isSoundStopped");
    }

    bool SoundManager::isMusicStopped(MusicHandle handle) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return checkAudioState(m_activeMusic, handle, SoundState::Stopped, "isMusicStopped");
    }

    size_t SoundManager::getActiveSoundsCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeSounds.size();
    }

    size_t SoundManager::getActiveMusicCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeMusic.size();
    }

    SoundHandle SoundManager::fadeInSound(const std::string &filename, float duration,
                                          float volume, float pitch, bool loop, SoundPriority priority)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        SoundHandle handle = play(m_activeSounds, filename, 0.0f, pitch, loop, priority, m_nextSoundHandle, "fadeInSound");

        if (!isHandleValid(handle))
            return INVALID_SOUND_HANDLE;

        ActiveAudio &sound = m_activeSounds[handle];
        sound.m_baseVolume = volume;

        if (fade(m_activeSounds, handle, true, duration, "fadeInSound"))
            return handle;

        stopSound(handle);
        return INVALID_SOUND_HANDLE;
    }

    MusicHandle SoundManager::fadeInMusic(const std::string &filename, float duration,
                                          float volume, float pitch, bool loop, SoundPriority priority)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        MusicHandle handle = playMusic(filename, 0.0f, pitch, loop, priority);
        if (!isHandleValid(handle))
            return INVALID_MUSIC_HANDLE;

        ActiveAudio &music = m_activeMusic[handle];
        music.m_baseVolume = volume;

        if (fade(m_activeMusic, handle, true, duration, "fadeInMusic"))
            return handle;

        stopMusic(handle);
        return INVALID_MUSIC_HANDLE;
    }

    bool SoundManager::fadeOutSound(SoundHandle handle, float duration)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return fade(m_activeSounds, handle, false, duration, "fadeOutSound");
    }

    bool SoundManager::fadeOutMusic(MusicHandle handle, float duration)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return fade(m_activeMusic, handle, false, duration, "fadeOutMusic");
    }

    bool SoundManager::fadeToVolumeSound(SoundHandle handle, float targetVolume, float duration)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return fadeToVolume(m_activeSounds, handle, targetVolume, duration, "fadeToVolumeSound");
    }

    bool SoundManager::fadeToVolumeMusic(MusicHandle handle, float targetVolume, float duration)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return fadeToVolume(m_activeMusic, handle, targetVolume, duration, "fadeToVolumeMusic");
    }

    bool SoundManager::setMasterVolume(float volume)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterVolume = volume;
        updateAllVolume();

        return true;
    }

    bool SoundManager::setMasterSoundsVolume(float volume)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterSoundsVolume = volume;
        updateAllSoundsVolume();

        return true;
    }

    bool SoundManager::setMasterMusicVolume(float volume)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterMusicVolume = volume;
        updateAllMusicVolume();

        return true;
    }

    bool SoundManager::setMasterPitch(float pitch)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterPitch = pitch;
        updateAllPitch();

        return true;
    }

    bool SoundManager::setMasterSoundsPitch(float pitch)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterSoundsPitch = pitch;
        updateAllSoundsPitch();

        return true;
    }

    bool SoundManager::setMasterMusicPitch(float pitch)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_masterMusicPitch = pitch;
        updateAllMusicPitch();

        return true;
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
        updateAllSoundsVolume();

        return true;
    }

    bool SoundManager::muteAllMusic()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_musicMute = true;
        updateAllMusicVolume();

        return true;
    }

    bool SoundManager::muteAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_mute = true;
        updateAllVolume();

        return true;
    }

    bool SoundManager::unmuteAllSounds()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_soundsMute = false;
        updateAllSoundsVolume();

        return true;
    }

    bool SoundManager::unmuteAllMusic()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_musicMute = false;
        updateAllMusicVolume();

        return true;
    }

    bool SoundManager::unmuteAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_mute = false;
        updateAllVolume();

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
        m_hasError = false;
        return error;
    }

    void SoundManager::clearError()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lastError = "";
        m_hasError = false;
    }

    bool SoundManager::isHandleValid(SoundHandle handle) { return handle != INVALID_SOUND_HANDLE; }

    bool SoundManager::isHandleValid(MusicHandle handle) { return handle != INVALID_MUSIC_HANDLE; }

} // namespace soundcoe