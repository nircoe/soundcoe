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
        class AudioContext
        {
            ALCdevice *m_device     = nullptr;
            ALCcontext *m_context   = nullptr;
            bool m_initialized      = false;
            mutable std::mutex m_mutex;

            AudioContext(const AudioContext &) = delete;
            AudioContext &operator=(const AudioContext &) = delete;
            AudioContext(AudioContext &&) = delete;
            AudioContext &operator=(AudioContext &&) = delete;

        public:
            AudioContext();
            ~AudioContext();

            void initialize(const std::string &deviceName = "");
            void shutdown();

            bool isInitialized() const;
            ALCdevice *getDevice() const;
            ALCcontext *getContext() const;
        };
    } // namespace detail
} // namespace soundcoe