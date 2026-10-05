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
            ALCdevice *m_device     = nullptr;
            ALCcontext *m_context   = nullptr;
            bool m_initialized      = false;
            mutable std::mutex m_mutex;

            audio_context(const audio_context &) = delete;
            audio_context &operator=(const audio_context &) = delete;
            audio_context(audio_context &&) = delete;
            audio_context &operator=(audio_context &&) = delete;

        public:
            audio_context();
            ~audio_context();

            [[nodiscard]] std::expected<void, error> initialize(const std::string &device_name = "");
            [[nodiscard]] std::expected<void, error> shutdown();

            bool is_initialized() const;
            ALCdevice *get_device() const;
            ALCcontext *get_context() const;
        };
    } // namespace internal
} // namespace soundcoe
