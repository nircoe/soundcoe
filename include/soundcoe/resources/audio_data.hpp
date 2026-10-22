#pragma once

#include <soundcoe/core/error.hpp>
#include <soundcoe/core/types.hpp>
#include <AL/al.h>
#include <string>
#include <expected>

namespace soundcoe
{
    namespace internal
    {
        class audio_data
        {
            ALvoid *m_pcm_data = nullptr;
            ALsizei m_pcm_data_size = 0;

            ALsizei m_channels = 0;
            ALsizei m_bits_per_sample = 0;
            ALsizei m_sample_rate = 0;
            ALfloat m_duration = 0.0f;

            ALenum m_openal_format = AL_NONE;
            audio_format m_source_format = audio_format::unsupported;

            audio_data(ALvoid *pcm_data, ALsizei pcm_data_size, ALsizei channels, ALsizei bits_per_sample, ALsizei sample_rate,
                audio_format source_format);

            void cleanup();

            [[nodiscard]] ALenum calculate_openal_format(ALsizei channels, ALsizei bits_per_sample);

            [[nodiscard]] static bool is_valid_wav(const std::string &filename);
            [[nodiscard]] static bool is_valid_mp3(const std::string &filename);
            [[nodiscard]] static bool is_valid_ogg(const std::string &filename);

        public:
            audio_data();
            audio_data(const audio_data &) = delete;
            audio_data &operator=(const audio_data &) = delete;
            audio_data(audio_data &&other) noexcept;
            audio_data &operator=(audio_data &&other) noexcept;
            ~audio_data();

            [[nodiscard]] static std::expected<audio_data, error> load_from_wav(const std::string &filename);
            [[nodiscard]] static std::expected<audio_data, error> load_from_ogg(const std::string &filename);
            [[nodiscard]] static std::expected<audio_data, error> load_from_mp3(const std::string &filename);
            [[nodiscard]] static audio_format detect_format(const std::string &filename);

            [[nodiscard]] ALvoid *get_pcm_data() const noexcept;
            [[nodiscard]] ALsizei get_pcm_data_size() const noexcept;
            [[nodiscard]] ALsizei get_channels() const noexcept;
            [[nodiscard]] ALsizei get_bits_per_sample() const noexcept;
            [[nodiscard]] ALsizei get_sample_rate() const noexcept;
            [[nodiscard]] ALfloat get_duration() const noexcept;
            [[nodiscard]] ALenum get_openal_format() const noexcept;
            [[nodiscard]] audio_format get_source_format() const noexcept;

            [[nodiscard]] ALboolean is_valid() const noexcept;
        };
    } // namespace internal
} // namespace soundcoe
