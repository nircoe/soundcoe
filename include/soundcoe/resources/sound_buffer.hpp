#pragma once

#include <soundcoe/resources/audio_data.hpp>
#include <string>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    namespace internal
    {
        class sound_buffer
        {
            ALuint m_buffer_id       = 0;
            ALenum m_format         = 0;
            ALsizei m_size          = 0;
            ALsizei m_sample_rate    = 0;
            ALfloat m_duration      = 0.0f;
            bool m_loaded           = false;
            bool m_stream           = false;
            std::string m_filename = "";

            void load_from_audio_data(audio_data &&audio_data_);
            void generate_buffer(const void* data);

        public:
            sound_buffer();
            sound_buffer(const std::string &filename);
            sound_buffer(const void *data, ALenum format, ALsizei size, ALsizei sample_rate);
            ~sound_buffer();

            sound_buffer(const sound_buffer &) = delete;
            sound_buffer &operator=(const sound_buffer &) = delete;
            sound_buffer(sound_buffer &&other) noexcept;
            sound_buffer &operator=(sound_buffer &&other) noexcept;

            void load_from_file(const std::string &filename);
            void load_from_memory(const void *data, ALenum format, ALsizei size, ALsizei sample_rate);
            void unload();

            ALuint get_buffer_id() const;
            ALenum get_format() const;
            ALsizei get_size() const;
            ALsizei get_sample_rate() const;
            ALfloat get_duration() const;
            bool is_loaded() const;
            bool is_streaming() const;
            const std::string &get_filename() const;
        };
    } // namespace internal
} // namespace soundcoe
