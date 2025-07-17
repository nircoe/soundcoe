#pragma once

#include <soundcoe/core/types.hpp>
#include <soundcoe/resources/sound_buffer.hpp>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    namespace detail
    {
        class SoundSource
        {
            ALuint m_sourceId       = 0;
            ALfloat m_volume        = 1.0f;
            ALfloat m_pitch         = 1.0f;
            Vec3 m_position;
            Vec3 m_velocity;
            ALboolean m_looping     = AL_FALSE;
            bool m_created          = false;

        public:
            SoundSource();
            SoundSource(const SoundBuffer &buffer);
            SoundSource(const SoundSource &) = delete;
            SoundSource& operator=(const SoundSource &) = delete;
            SoundSource(SoundSource &&other) noexcept;
            SoundSource &operator=(SoundSource &&other) noexcept;
            ~SoundSource();

            void create();
            void destroy();
            bool isCreated() const;

            void attachBuffer(const SoundBuffer &buffer);
            void detachBuffer();

            bool play();
            bool pause();
            bool stop();

            bool setVolume(float volume);
            bool setPitch(float pitch);
            bool setPosition(const Vec3 &position);
            bool setVelocity(const Vec3 &velocity);
            bool setLooping(bool looping);
            float getVolume() const;
            float getPitch() const;
            const Vec3 &getPosition() const;
            const Vec3 &getVelocity() const;
            bool isLooping() const;

            SoundState getState() const;
            bool isPlaying() const;
            bool isPaused() const;
            bool isStopped() const;

            ALuint getSourceId() const;
            ALuint getBufferId() const;
        };
    } // namespace detail
} // namespace soundcoe