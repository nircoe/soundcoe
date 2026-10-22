#pragma once

#include <soundcoe/core/error.hpp>
#include <soundcoe/resources/audio_data.hpp>
#include <AL/al.h>
#include <expected>
#include <string>

namespace soundcoe
{
    namespace internal
    {
        class sound_buffer
        {
            std::string m_filename;
            ALuint m_buffer_id = 0;
            ALenum m_format = 0;
            ALsizei m_size = 0;
            ALsizei m_sample_rate = 0;
            ALfloat m_duration = 0.0f;
            bool m_loaded = false;
            bool m_stream = false;

            [[nodiscard]] std::expected<void, error> load_from_audio_data(audio_data &&audio_data_);
            [[nodiscard]] std::expected<void, error> generate_buffer(const void* data);

        public:
            sound_buffer();
            ~sound_buffer();

            sound_buffer(const sound_buffer &) = delete;
            sound_buffer &operator=(const sound_buffer &) = delete;
            sound_buffer(sound_buffer &&other) noexcept;
            sound_buffer &operator=(sound_buffer &&other) noexcept;

            [[nodiscard]] std::expected<void, error> load_from_file(const std::string &filename);
            [[nodiscard]] std::expected<void, error> load_from_memory(const void *data, ALenum format, ALsizei size,
                                                                        ALsizei sample_rate);
            void unload();

            [[nodiscard]] ALuint get_buffer_id() const noexcept;
            [[nodiscard]] ALenum get_format() const noexcept;
            [[nodiscard]] ALsizei get_size() const noexcept;
            [[nodiscard]] ALsizei get_sample_rate() const noexcept;
            [[nodiscard]] ALfloat get_duration() const noexcept;
            [[nodiscard]] bool is_loaded() const noexcept;
            [[nodiscard]] bool is_streaming() const noexcept;
            [[nodiscard]] const std::string &get_filename() const noexcept;
        };
    } // namespace internal
} // namespace soundcoe
