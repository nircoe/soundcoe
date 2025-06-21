#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <utility>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    class AudioContext
    {
        ALCdevice *m_device;
        ALCcontext *m_context;
        bool m_initialized;
        mutable std::mutex m_mutex;

        AudioContext();

        AudioContext(const AudioContext &) = delete;
        AudioContext &operator=(const AudioContext &) = delete;
        AudioContext(AudioContext &&) = delete;
        AudioContext &operator=(AudioContext &&) = delete;

        bool initialize(const std::string &deviceName = "");

    public:
        ~AudioContext();

        void shutdown();

        bool isInitialized() const;
        ALCdevice *getDevice() const;
        ALCcontext *getContext() const;

        static AudioContext &getInstance(const std::string &deviceName = "");
    };
} // namespace soundcoe