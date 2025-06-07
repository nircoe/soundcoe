#pragma once

#include <string>
#include <memory>
#include <optional>
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

        static std::optional<AudioContext> s_instance;

        AudioContext();
        ~AudioContext();

        AudioContext(const AudioContext &) = delete;
        AudioContext &operator=(const AudioContext &) = delete;
        AudioContext(AudioContext &&) = delete;
        AudioContext &operator=(AudioContext &&) = delete;

    public:
        bool initialize(const std::string &deviceName = "");
        void shutdown();

        bool isInitialized() const;
        ALCdevice *getDevice() const;
        ALCcontext *getContext() const;

        static AudioContext& getInstance();
    };
} // namespace soundcoe