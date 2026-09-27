#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <utility>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    namespace detail
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

            void initialize(const std::string &device_name = "");
            void shutdown();

            bool is_initialized() const;
            ALCdevice *get_device() const;
            ALCcontext *get_context() const;
        };
    } // namespace detail
} // namespace soundcoe
