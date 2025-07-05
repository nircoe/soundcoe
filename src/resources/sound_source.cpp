#include <soundcoe/resources/sound_source.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <exception>
#include <logcoe.hpp>

namespace soundcoe
{
    SoundSource::SoundSource() : m_sourceId(0), m_volume(1.0f), m_pitch(1.0f), m_position(Vec3::zero()),
                                 m_velocity(Vec3::zero()), m_looping(AL_FALSE), m_created(false) { }

    SoundSource::SoundSource(const SoundBuffer &buffer)
    {
        create();
        attachBuffer(buffer);
    }

    SoundSource::SoundSource(SoundSource &&other) noexcept :
        m_sourceId(other.m_sourceId), m_volume(other.m_volume), m_pitch(other.m_pitch), m_position(other.m_position),
        m_velocity(other.m_velocity), m_looping(other.m_looping), m_created(other.m_created)
    {
        other.m_sourceId = 0;
        other.m_volume = 1.0f;
        other.m_pitch = 1.0f;
        other.m_position = Vec3::zero();
        other.m_velocity = Vec3::zero();
        other.m_looping = AL_FALSE;
        other.m_created = false;
    }

    SoundSource &SoundSource::operator=(SoundSource &&other) noexcept
    {
        if(this == &other) return *this;

        destroy();

        m_sourceId = other.m_sourceId;
        m_volume = other.m_volume;
        m_pitch = other.m_pitch;
        m_position = other.m_position;
        m_velocity = other.m_velocity;
        m_looping = other.m_looping;
        m_created = other.m_created;

        other.m_sourceId = 0;
        other.m_volume = 1.0f;
        other.m_pitch = 1.0f;
        other.m_position = Vec3::zero();
        other.m_velocity = Vec3::zero();
        other.m_looping = AL_FALSE;
        other.m_created = false;
        return *this;
    }

    SoundSource::~SoundSource() { destroy(); }

    void SoundSource::create()
    {
        if(m_created) return;

        alGenSources(1, &m_sourceId);
        ErrorHandler::throwOnOpenALError("Generate Source");

        m_created = true;
        
        setVolume(1.0f);
        setPitch(1.0f);
        setLooping(false);
    }

    void SoundSource::destroy()
    {
        if(!m_created) return;

        if(isPlaying() || isPaused()) stop();

        try { detachBuffer(); }
        catch(...) { }

        alDeleteSources(1, &m_sourceId);
        ErrorHandler::throwOnOpenALError("Delete Source");

        m_sourceId = 0;
        m_created = false;
    }

    bool SoundSource::isCreated() const
    {
        return m_created;
    }

    void SoundSource::attachBuffer(const SoundBuffer &buffer)
    {   
        if(!m_created) create();

        ALint bufferId;
        alGetSourcei(m_sourceId, AL_BUFFER, &bufferId);
        if (bufferId != 0) detachBuffer();
        
        alSourcei(m_sourceId, AL_BUFFER, static_cast<ALint>(buffer.getBufferId()));
        ErrorHandler::throwOnOpenALError("Attach Buffer to Source");
    }

    void SoundSource::detachBuffer()
    {
        if(!m_created) return;

        if(isPlaying() || isPaused()) stop();

        alSourcei(m_sourceId, AL_BUFFER, 0);
        ErrorHandler::throwOnOpenALError("Detach Buffer from Source");
    }

    bool SoundSource::play()
    {
        if(!m_created)
        {
            logcoe::warning("SoundSource not created");
            return false;
        }

        if(isPlaying())
        {
            logcoe::debug("SoundSource is already playing");
            return true;
        }

        alSourcePlay(m_sourceId);
        if(ErrorHandler::checkOpenALError("Play Source"))
            return false;
        
        return true;
    }

    bool SoundSource::pause()
    {
        if(!m_created)
        {
            logcoe::warning("SoundSource not created");
            return false;
        }

        if(isPaused())
        {
            logcoe::debug("SoundSource is already paused");
            return true;
        }

        alSourcePause(m_sourceId);
        if(ErrorHandler::checkOpenALError("Pause Source"))
            return false;

        return true;
    }

    bool SoundSource::stop()
    {
        if(!m_created)
        {
            logcoe::warning("SoundSource not created");
            return false;
        }

        if(!(isPlaying() || isPaused()))
        {
            logcoe::debug("SoundSource is already stopped or in initial state");
            return true;
        }

        alSourceStop(m_sourceId);
        if(ErrorHandler::checkOpenALError("Stop Source"))
            return false;

        return true;
    }

    bool SoundSource::setVolume(float volume)
    {
        if(!m_created)
        {
            logcoe::warning("SoundSource not created");
            return false;
        }
        ALfloat ALvolume = static_cast<ALfloat>(volume);
        alSourcef(m_sourceId, AL_GAIN, ALvolume);
        if (ErrorHandler::checkOpenALError("Set Volume"))
            return false;

        m_volume = ALvolume;
        return true;
    }

    bool SoundSource::setPitch(float pitch)
    {
        if(!m_created)
        {
            logcoe::warning("SoundSource not created");
            return false;
        }
        ALfloat ALpitch = static_cast<ALfloat>(pitch);
        alSourcef(m_sourceId, AL_PITCH, ALpitch);
        if (ErrorHandler::checkOpenALError("Set Pitch"))
            return false;

        m_pitch = ALpitch;
        return true;
    }

    bool SoundSource::setPosition(const Vec3 &position)
    {
        if(!m_created)
        {
            logcoe::warning("SoundSource not created");
            return false;
        }
        alSource3f(m_sourceId, AL_POSITION,
                   static_cast<ALfloat>(position.x), static_cast<ALfloat>(position.y), static_cast<ALfloat>(position.z));
        if (ErrorHandler::checkOpenALError("Set Position"))
            return false;

        m_position = position;
        return true;
    }

    bool SoundSource::setVelocity(const Vec3 &velocity)
    {
        if(!m_created)
        {
            logcoe::warning("SoundSource not created");
            return false;
        }
        alSource3f(m_sourceId, AL_VELOCITY,
                   static_cast<ALfloat>(velocity.x), static_cast<ALfloat>(velocity.y), static_cast<ALfloat>(velocity.z));
        if (ErrorHandler::checkOpenALError("Set Velocity"))
            return false;

        m_velocity = velocity;
        return true;
    }

    bool SoundSource::setLooping(bool looping)
    {
        if(!m_created)
        {
            logcoe::warning("SoundSource not created");
            return false;
        }
        ALboolean ALlooping = looping ? AL_TRUE : AL_FALSE;
        alSourcei(m_sourceId, AL_LOOPING, ALlooping);
        if (ErrorHandler::checkOpenALError("Set Looping"))
            return false;

        m_looping = ALlooping;
        return true;
    }

    float SoundSource::getVolume() const { return static_cast<float>(m_volume); }

    float SoundSource::getPitch() const { return static_cast<float>(m_pitch); }

    const Vec3 &SoundSource::getPosition() const { return m_position; }

    const Vec3 &SoundSource::getVelocity() const { return m_velocity; }

    bool SoundSource::isLooping() const { return static_cast<bool>(m_looping); }

    SoundState SoundSource::getState() const 
    { 
        if(!m_created) 
        {
            logcoe::warning("SoundSource not created");
            return SoundState::Initial;
        }

        ALint state;
        alGetSourcei(m_sourceId, AL_SOURCE_STATE, &state);
        if(ErrorHandler::checkOpenALError("Get Source State"))
            return SoundState::Initial;

        switch(state)
        {
            case AL_INITIAL: return SoundState::Initial;
            case AL_PLAYING: return SoundState::Playing;
            case AL_PAUSED: return SoundState::Paused;
            case AL_STOPPED: return SoundState::Stopped;
        }

        return SoundState::Initial;
    }

    bool SoundSource::isPlaying() const { return getState() == SoundState::Playing; }

    bool SoundSource::isPaused() const { return getState() == SoundState::Paused; }

    bool SoundSource::isStopped() const { return getState() == SoundState::Stopped; }

    ALuint SoundSource::getSourceId() const { return m_sourceId; }

    ALuint SoundSource::getBufferId() const
    {
        if(!m_created)
        {
            logcoe::warning("SoundSource not created");
            return 0;
        }

        ALint bufferId;
        alGetSourcei(m_sourceId, AL_BUFFER, &bufferId);
        if (ErrorHandler::checkOpenALError("Get Buffer Id"))
            return 0;

        return static_cast<ALuint>(bufferId);
    }

} // namespace soundcoe