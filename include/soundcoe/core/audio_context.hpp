#pragma once

#include <soundcoe/core/error.hpp>
#include <AL/alc.h>
#include <expected>
#include <mutex>
#include <string>

namespace soundcoe
{
    namespace internal
    {
        class audio_context
        {
            ALCdevice *m_device = nullptr;
            ALCcontext *m_context = nullptr;
            mutable std::mutex m_mutex;
            bool m_initialized = false;

            audio_context(const audio_context &) = delete;
            audio_context &operator=(const audio_context &) = delete;
            audio_context(audio_context &&) = delete;
            audio_context &operator=(audio_context &&) = delete;

        public:
            audio_context();
            ~audio_context();

            [[nodiscard]] std::expected<void, error> initialize(const std::string &device_name = "");
            [[nodiscard]] std::expected<void, error> shutdown();

            [[nodiscard]] bool is_initialized() const;
            [[nodiscard]] ALCdevice *get_device() const;
            [[nodiscard]] ALCcontext *get_context() const;
        };
    } // namespace internal
} // namespace soundcoe
