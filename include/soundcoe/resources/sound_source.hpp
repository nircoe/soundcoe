#pragma once

#include <expected>
#include <soundcoe/core/error.hpp>
#include <soundcoe/core/types.hpp>
#include <soundcoe/resources/sound_buffer.hpp>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    namespace internal
    {
        class sound_source
        {
            ALuint m_source_id       = 0;
            ALfloat m_volume        = 1.0f;
            ALfloat m_pitch         = 1.0f;
            vec3 m_position;
            vec3 m_velocity;
            ALboolean m_looping     = AL_FALSE;
            bool m_created          = false;

        public:
            sound_source();
            sound_source(const sound_source &) = delete;
            sound_source& operator=(const sound_source &) = delete;
            sound_source(sound_source &&other) noexcept;
            sound_source &operator=(sound_source &&other) noexcept;
            ~sound_source();

            [[nodiscard]] std::expected<void, error> create();
            [[nodiscard]] std::expected<void, error> destroy();
            bool is_created() const;

            [[nodiscard]] std::expected<void, error> attach_buffer(const sound_buffer &buffer);
            [[nodiscard]] std::expected<void, error> detach_buffer();

            [[nodiscard]] std::expected<void, error> play();
            [[nodiscard]] std::expected<void, error> pause();
            [[nodiscard]] std::expected<void, error> stop();

            [[nodiscard]] std::expected<void, error> set_volume(float volume);
            [[nodiscard]] std::expected<void, error> set_pitch(float pitch);
            [[nodiscard]] std::expected<void, error> set_position(const vec3 &position);
            [[nodiscard]] std::expected<void, error> set_velocity(const vec3 &velocity);
            [[nodiscard]] std::expected<void, error> set_looping(bool looping);
            float get_volume() const;
            float get_pitch() const;
            const vec3 &get_position() const;
            const vec3 &get_velocity() const;
            bool is_looping() const;

            sound_state get_state() const;
            bool is_playing() const;
            bool is_paused() const;
            bool is_stopped() const;

            ALuint get_source_id() const;
            ALuint get_buffer_id() const;
        };
    } // namespace internal
} // namespace soundcoe
