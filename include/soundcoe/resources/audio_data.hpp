#pragma once

#include <soundcoe/core/types.hpp>
#include <AL/al.h>
#include <string>

namespace soundcoe
{
    namespace internal
    {
        class audio_data
        {
            ALvoid *m_pcm_data;
            ALsizei m_pcm_data_size;

            ALsizei m_channels;
            ALsizei m_bits_per_sample;
            ALsizei m_sample_rate;
            ALfloat m_duration;

            ALenum m_openal_format;
            audio_format m_source_format;

            audio_data(ALvoid *pcm_data, ALsizei pcm_data_size, ALsizei channels, ALsizei bits_per_sample, ALsizei sample_rate,
                audio_format source_format);

            void cleanup();

            ALenum calculate_openal_format(ALsizei channels, ALsizei bits_per_sample);

            static bool is_valid_wav(const std::string &filename);
            static bool is_valid_mp3(const std::string &filename);
            static bool is_valid_ogg(const std::string &filename);

        public:
            audio_data();
            audio_data(const audio_data &) = delete;
            audio_data &operator=(const audio_data &) = delete;
            audio_data(audio_data &&other) noexcept;
            audio_data &operator=(audio_data &&other) noexcept;
            ~audio_data();

            static audio_data load_from_wav(const std::string &filename);
            static audio_data load_from_ogg(const std::string &filename);
            static audio_data load_from_mp3(const std::string &filename);
            static audio_format detect_format(const std::string &filename);

            ALvoid *get_pcm_data() const;
            ALsizei get_pcm_data_size() const;
            ALsizei get_channels() const;
            ALsizei get_bits_per_sample() const;
            ALsizei get_sample_rate() const;
            ALfloat get_duration() const;
            ALenum get_openal_format() const;
            audio_format get_source_format() const;

            ALboolean is_valid() const;
        };
    } // namespace internal
} // namespace soundcoe
