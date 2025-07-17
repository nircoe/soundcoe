#pragma once

#include <soundcoe/resources/audio_data.hpp>
#include <string>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    namespace detail
    {
        class SoundBuffer
        {
            ALuint m_bufferId       = 0;
            ALenum m_format         = 0;
            ALsizei m_size          = 0;
            ALsizei m_sampleRate    = 0;
            ALfloat m_duration      = 0.0f;
            bool m_loaded           = false;
            bool m_stream           = false;
            std::string m_filename = "";

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
            bool isStreaming() const;
            const std::string &getFileName() const;
        };
    } // namespace detail
} // namespace soundcoe