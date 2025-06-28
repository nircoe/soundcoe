#pragma once

#include <soundcoe/core/types.hpp>
#include <soundcoe/resources/sound_buffer.hpp>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    class SoundSource
    {
        ALuint m_sourceId;
        ALfloat m_volume;
        ALfloat m_pitch;
        Vec3 m_position;
        Vec3 m_velocity;
        ALboolean m_looping;
        bool m_created;

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

} // namespace soundcoe