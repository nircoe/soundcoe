#include <soundcoe/resources/sound_source.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <exception>
#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace detail
    {
        sound_source::sound_source() : m_position(vec3::zero()), m_velocity(vec3::zero()) { }

        sound_source::sound_source(const sound_buffer &buffer)
        {
            create();
            attach_buffer(buffer);
        }

        sound_source::sound_source(sound_source &&other) noexcept :
            m_source_id(other.m_source_id), m_volume(other.m_volume), m_pitch(other.m_pitch), m_position(other.m_position),
            m_velocity(other.m_velocity), m_looping(other.m_looping), m_created(other.m_created)
        {
            other.m_source_id = 0;
            other.m_volume = 1.0f;
            other.m_pitch = 1.0f;
            other.m_position = vec3::zero();
            other.m_velocity = vec3::zero();
            other.m_looping = AL_FALSE;
            other.m_created = false;
        }

        sound_source &sound_source::operator=(sound_source &&other) noexcept
        {
            if(this == &other) return *this;

            destroy();

            m_source_id = other.m_source_id;
            m_volume = other.m_volume;
            m_pitch = other.m_pitch;
            m_position = other.m_position;
            m_velocity = other.m_velocity;
            m_looping = other.m_looping;
            m_created = other.m_created;

            other.m_source_id = 0;
            other.m_volume = 1.0f;
            other.m_pitch = 1.0f;
            other.m_position = vec3::zero();
            other.m_velocity = vec3::zero();
            other.m_looping = AL_FALSE;
            other.m_created = false;
            return *this;
        }

        sound_source::~sound_source() { destroy(); }

        void sound_source::create()
        {
            if(m_created)
            {
                logcoe::info("sound_source::create: sound_source is already created");
                return;
            }

            alGenSources(1, &m_source_id);
            error_handler::throw_on_openal_error("Generate Source");

            m_created = true;

            set_volume(1.0f);
            set_pitch(1.0f);
            set_looping(false);

            logcoe::info("sound_source::create: sound_source created successfully");
        }

        void sound_source::destroy()
        {
            if(!m_created) return;

            if(is_playing() || is_paused()) stop();

            try { detach_buffer(); }
            catch(...) { }

            alDeleteSources(1, &m_source_id);
            error_handler::throw_on_openal_error("Delete Source");

            m_source_id = 0;
            m_created = false;
        }

        bool sound_source::is_created() const
        {
            return m_created;
        }

        void sound_source::attach_buffer(const sound_buffer &buffer)
        {
            if(!m_created) create();

            ALint buffer_id;
            alGetSourcei(m_source_id, AL_BUFFER, &buffer_id);
            if (buffer_id != 0) detach_buffer();

            alSourcei(m_source_id, AL_BUFFER, static_cast<ALint>(buffer.get_buffer_id()));
            error_handler::throw_on_openal_error("Attach Buffer to Source");
        }

        void sound_source::detach_buffer()
        {
            if(!m_created) return;

            if(is_playing() || is_paused()) stop();

            alSourcei(m_source_id, AL_BUFFER, 0);
            error_handler::throw_on_openal_error("Detach Buffer from Source");
        }

        bool sound_source::play()
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::play: sound_source not created");
                return false;
            }

            if(is_playing())
            {
                logcoe::debug("sound_source::play: sound_source is already playing");
                return true;
            }

            alSourcePlay(m_source_id);
            if(error_handler::check_openal_error("Play Source"))
                return false;

            return true;
        }

        bool sound_source::pause()
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::pause: sound_source not created");
                return false;
            }

            if(is_paused())
            {
                logcoe::debug("sound_source::pause: sound_source is already paused");
                return true;
            }

            alSourcePause(m_source_id);
            if(error_handler::check_openal_error("Pause Source"))
                return false;

            return true;
        }

        bool sound_source::stop()
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::stop: sound_source not created");
                return false;
            }

            if(!(is_playing() || is_paused()))
            {
                logcoe::debug("sound_source::stop: sound_source is already stopped or in initial state");
                return true;
            }

            alSourceStop(m_source_id);
            if(error_handler::check_openal_error("Stop Source"))
                return false;

            return true;
        }

        bool sound_source::set_volume(float volume)
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::set_volume: sound_source not created");
                return false;
            }
            ALfloat al_volume = static_cast<ALfloat>(volume);
            alSourcef(m_source_id, AL_GAIN, al_volume);
            if (error_handler::check_openal_error("Set Volume"))
                return false;

            m_volume = al_volume;
            return true;
        }

        bool sound_source::set_pitch(float pitch)
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::set_pitch: sound_source not created");
                return false;
            }
            ALfloat al_pitch = static_cast<ALfloat>(pitch);
            alSourcef(m_source_id, AL_PITCH, al_pitch);
            if (error_handler::check_openal_error("Set Pitch"))
                return false;

            m_pitch = al_pitch;
            return true;
        }

        bool sound_source::set_position(const vec3 &position)
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::set_position: sound_source not created");
                return false;
            }
            alSource3f(m_source_id, AL_POSITION,
                    static_cast<ALfloat>(position.x), static_cast<ALfloat>(position.y), static_cast<ALfloat>(position.z));
            if (error_handler::check_openal_error("Set Position"))
                return false;

            m_position = position;
            return true;
        }

        bool sound_source::set_velocity(const vec3 &velocity)
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::set_velocity: sound_source not created");
                return false;
            }
            alSource3f(m_source_id, AL_VELOCITY,
                    static_cast<ALfloat>(velocity.x), static_cast<ALfloat>(velocity.y), static_cast<ALfloat>(velocity.z));
            if (error_handler::check_openal_error("Set Velocity"))
                return false;

            m_velocity = velocity;
            return true;
        }

        bool sound_source::set_looping(bool looping)
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::set_looping: sound_source not created");
                return false;
            }
            ALboolean al_looping = looping ? AL_TRUE : AL_FALSE;
            alSourcei(m_source_id, AL_LOOPING, al_looping);
            if (error_handler::check_openal_error("Set Looping"))
                return false;

            m_looping = al_looping;
            return true;
        }

        float sound_source::get_volume() const { return static_cast<float>(m_volume); }

        float sound_source::get_pitch() const { return static_cast<float>(m_pitch); }

        const vec3 &sound_source::get_position() const { return m_position; }

        const vec3 &sound_source::get_velocity() const { return m_velocity; }

        bool sound_source::is_looping() const { return static_cast<bool>(m_looping); }

        sound_state sound_source::get_state() const
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::get_state: sound_source not created");
                return sound_state::initial;
            }

            ALint state;
            alGetSourcei(m_source_id, AL_SOURCE_STATE, &state);
            if(error_handler::check_openal_error("Get Source State"))
                return sound_state::initial;

            switch(state)
            {
                case AL_INITIAL: return sound_state::initial;
                case AL_PLAYING: return sound_state::playing;
                case AL_PAUSED: return sound_state::paused;
                case AL_STOPPED: return sound_state::stopped;
            }

            return sound_state::initial;
        }

        bool sound_source::is_playing() const { return get_state() == sound_state::playing; }

        bool sound_source::is_paused() const { return get_state() == sound_state::paused; }

        bool sound_source::is_stopped() const { return get_state() == sound_state::stopped; }

        ALuint sound_source::get_source_id() const { return m_source_id; }

        ALuint sound_source::get_buffer_id() const
        {
            if(!m_created)
            {
                logcoe::warning("sound_source::get_buffer_id: sound_source not created");
                return 0;
            }

            ALint buffer_id;
            alGetSourcei(m_source_id, AL_BUFFER, &buffer_id);
            if (error_handler::check_openal_error("Get Buffer Id"))
                return 0;

            return static_cast<ALuint>(buffer_id);
        }
    } // namespace detail
} // namespace soundcoe