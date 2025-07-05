#pragma once

#include <soundcoe/resources/audio_data.hpp>
#include <string>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    class SoundBuffer
    {
        ALuint m_bufferId;
        ALenum m_format;
        ALsizei m_size;
        ALsizei m_sampleRate;
        ALfloat m_duration;
        bool m_loaded;
        std::string m_filename;

        void loadFromAudioData(AudioData &&audioData);
        void generateBuffer(const void* data);

    public:
        SoundBuffer();
        SoundBuffer(const std::string &filename);
        SoundBuffer(const void *data, ALenum format, ALsizei size, ALsizei sampleRate);
        ~SoundBuffer();

        SoundBuffer(const SoundBuffer &) = delete;
        SoundBuffer &operator=(const SoundBuffer &) = delete;
        SoundBuffer(SoundBuffer &&other) noexcept;
        SoundBuffer &operator=(SoundBuffer &&other) noexcept;

        void loadFromFile(const std::string &filename);
        void loadFromMemory(const void *data, ALenum format, ALsizei size, ALsizei sampleRate);
        void unload();

        ALuint getBufferId() const;
        ALenum getFormat() const;
        ALsizei getSize() const;
        ALsizei getSampleRate() const;
        ALfloat getDuration() const;
        bool isLoaded() const;
        const std::string &getFileName() const;
    };
} // namespace soundcoe