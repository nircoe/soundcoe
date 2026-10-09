#include <soundcoe/resources/sound_source.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <soundcoe_config.hpp>
#include <expected>
#include <format>
#include <string_view>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace internal
    {
        namespace
        {
            [[nodiscard]] std::unexpected<error> not_created(std::string_view method)
            {
                return std::unexpected(error_handler::make_error(
                    error_code::invalid_state, std::format("sound_source::{}: sound_source not created", method)));
            }
        } // namespace

        sound_source::sound_source() : m_position(vec3::zero()), m_velocity(vec3::zero()) { }

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

            static_cast<void>(destroy());

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

        sound_source::~sound_source()
        {
            static_cast<void>(destroy());
        }

        std::expected<void, error> sound_source::create()
        {
            if(m_created)
            {
                logcoe::info("sound_source::create: sound_source is already created");
                return {};
            }

            alGenSources(1, &m_source_id);
            if(auto r = error_handler::check_openal_error("Generate Source"); !r)
                return r;

            m_created = true;

            if(auto r = set_volume(1.0f); !r)
                return r;
            if(auto r = set_pitch(1.0f); !r)
                return r;
            if(auto r = set_looping(false); !r)
                return r;

            logcoe::info("sound_source::create: sound_source created successfully");
            return {};
        }

        std::expected<void, error> sound_source::destroy()
        {
            if(!m_created) return {};

            if(is_playing() || is_paused()) static_cast<void>(stop());
            static_cast<void>(detach_buffer());

            alDeleteSources(1, &m_source_id);
            auto r = error_handler::check_openal_error("Delete Source");

            // A failed delete can't be retried, so reset anyway
            m_source_id = 0;
            m_created = false;
            return r;
        }

        bool sound_source::is_created() const
        {
            return m_created;
        }

        std::expected<void, error> sound_source::attach_buffer(const sound_buffer &buffer)
        {
            if(!m_created)
            {
                if(auto r = create(); !r)
                    return r;
            }

            ALint buffer_id;
            alGetSourcei(m_source_id, AL_BUFFER, &buffer_id);
            if(buffer_id != 0)
            {
                if(auto r = detach_buffer(); !r)
                    return r;
            }

            alSourcei(m_source_id, AL_BUFFER, static_cast<ALint>(buffer.get_buffer_id()));
            return error_handler::check_openal_error("Attach Buffer to Source");
        }

        std::expected<void, error> sound_source::detach_buffer()
        {
            if(!m_created) return {};

            if(is_playing() || is_paused()) static_cast<void>(stop());

            alSourcei(m_source_id, AL_BUFFER, 0);
            return error_handler::check_openal_error("Detach Buffer from Source");
        }

        std::expected<void, error> sound_source::play()
        {
            if (!m_created)
                return not_created("play");

            if(is_playing())
            {
                logcoe::debug("sound_source::play: sound_source is already playing");
                return {};
            }

            alSourcePlay(m_source_id);
            return error_handler::check_openal_error("Play Source");
        }

        std::expected<void, error> sound_source::pause()
        {
            if (!m_created)
                return not_created("pause");

            if(is_paused())
            {
                logcoe::debug("sound_source::pause: sound_source is already paused");
                return {};
            }

            alSourcePause(m_source_id);
            return error_handler::check_openal_error("Pause Source");
        }

        std::expected<void, error> sound_source::stop()
        {
            if (!m_created)
                return not_created("stop");

            if(!(is_playing() || is_paused()))
            {
                logcoe::debug("sound_source::stop: sound_source is already stopped or in initial state");
                return {};
            }

            alSourceStop(m_source_id);
            return error_handler::check_openal_error("Stop Source");
        }

        std::expected<void, error> sound_source::set_volume(float volume)
        {
            if (!m_created)
                return not_created("set_volume");

            ALfloat al_volume = static_cast<ALfloat>(volume);
            alSourcef(m_source_id, AL_GAIN, al_volume);
            if(auto r = error_handler::check_openal_error("Set Volume"); !r)
                return r;

            m_volume = al_volume;
            return {};
        }

        std::expected<void, error> sound_source::set_pitch(float pitch)
        {
            if (!m_created)
                return not_created("set_pitch");

            ALfloat al_pitch = static_cast<ALfloat>(pitch);
            alSourcef(m_source_id, AL_PITCH, al_pitch);
            if(auto r = error_handler::check_openal_error("Set Pitch"); !r)
                return r;

            m_pitch = al_pitch;
            return {};
        }

        std::expected<void, error> sound_source::set_position(const vec3 &position)
        {
            if (!m_created)
                return not_created("set_position");

            alSource3f(m_source_id, AL_POSITION,
                    static_cast<ALfloat>(position.x), static_cast<ALfloat>(position.y), static_cast<ALfloat>(position.z));
            if(auto r = error_handler::check_openal_error("Set Position"); !r)
                return r;

            m_position = position;
            return {};
        }

        std::expected<void, error> sound_source::set_velocity(const vec3 &velocity)
        {
            if (!m_created)
                return not_created("set_velocity");

            alSource3f(m_source_id, AL_VELOCITY,
                    static_cast<ALfloat>(velocity.x), static_cast<ALfloat>(velocity.y), static_cast<ALfloat>(velocity.z));
            if(auto r = error_handler::check_openal_error("Set Velocity"); !r)
                return r;

            m_velocity = velocity;
            return {};
        }

        std::expected<void, error> sound_source::set_looping(bool looping)
        {
            if (!m_created)
                return not_created("set_looping");

            ALboolean al_looping = looping ? AL_TRUE : AL_FALSE;
            alSourcei(m_source_id, AL_LOOPING, al_looping);
            if(auto r = error_handler::check_openal_error("Set Looping"); !r)
                return r;

            m_looping = al_looping;
            return {};
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
            if(!error_handler::check_openal_error("Get Source State"))
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
            if(!error_handler::check_openal_error("Get Buffer Id"))
                return 0;

            return static_cast<ALuint>(buffer_id);
        }
    } // namespace internal
} // namespace soundcoe
