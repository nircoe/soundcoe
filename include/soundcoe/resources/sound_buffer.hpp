#pragma once

#include <string>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    class SoundBuffer
    {
        ALuint m_bufferId;
        std::string m_filename;
        bool m_loaded;

        ALenum m_format;
        ALsizei m_size;
        ALsizei m_samplerate;
        float m_duration;

        bool loadWavFile();
        bool loadOggFile();
        bool loadMp3File();

    public:
        SoundBuffer();
        SoundBuffer(const std::string &filename);
        SoundBuffer(const void *data, ALsizei size, ALenum format, ALsizei samplerate);
        ~SoundBuffer();

        SoundBuffer(const SoundBuffer &) = delete;
        SoundBuffer &operator=(const SoundBuffer &) = delete;
        SoundBuffer(SoundBuffer &&other) noexcept;
        SoundBuffer &operator=(SoundBuffer &&other) noexcept;

        bool loadFromFile(const std::string &filename);
        bool loadFromMemory(const void *data, ALsizei size, ALenum format, ALsizei samplerate);
        void unload();

        ALuint getBufferId() const;
        const std::string &getFileName() const;
        bool isLoaded() const;
        ALenum getFormat() const;
        ALsizei getSize() const;
        ALsizei getSampleRate() const;
        float getDuration() const;

    };
} // namespace soundcoe