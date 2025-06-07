#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <logcoe.hpp>
#include <iostream>

namespace soundcoe
{
    std::optional<AudioContext> AudioContext::s_instance;

    AudioContext::AudioContext() :
        m_device(nullptr),
        m_context(nullptr),
        m_initialized(false) { }

    AudioContext::~AudioContext() { shutdown(); }

    bool AudioContext::initialize(const std::string& deviceName)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_initialized)
            return true;

        logcoe::initialize(logcoe::LogLevel::INFO, "soundcoe");

        logcoe::debug("Initializing ALCdevice");
        m_device = alcOpenDevice(deviceName.empty() ? 
                    nullptr : 
                    deviceName.c_str());
        if (!m_device)
        {
            ErrorHandler::checkALCError(nullptr, "Open Audio Device: \"" + deviceName + "\"");
            logcoe::error("Failed to initialize ALCdevice");
            return false;
        }

        logcoe::info("Initializing AudioContext");
        m_context = alcCreateContext(m_device, nullptr);
        if(!m_context)
        {
            ErrorHandler::checkALCError(m_device, "Create Audio Context");
            logcoe::error("Failed to initialize AudioContext");
            alcCloseDevice(m_device);
            m_device = nullptr;
            return false;
        }

        logcoe::debug("Make AudioContext current");
        if(!alcMakeContextCurrent(m_context))
        {
            ErrorHandler::checkALCError(m_device, "Make Context Current");
            logcoe::error("Failed to make context current");
            alcDestroyContext(m_context);
            alcCloseDevice(m_device);
            m_device = nullptr;
            m_context = nullptr;
            return false;
        }

        m_initialized = true;
        logcoe::info("AudioContext initialized successfully");
        return true;
    }

    void AudioContext::shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(!m_initialized)
            return;

        logcoe::info("Shutting down AudioContext");

        if(!alcMakeContextCurrent(nullptr) && ErrorHandler::checkALCError(m_device, "Make Context Current NULL"))
            logcoe::error("Failed to clear current context during shutdown, attempting to continue cleanup");
        
        if(m_context)
        {
            alcDestroyContext(m_context);
            if(ErrorHandler::checkALCError(m_device, "Destroy Context"))
                logcoe::error("Failed to destory OpenAL context during shutdown, attempting to continue cleanup");
            m_context = nullptr;
        }

        if(m_device)
        {
            if(!alcCloseDevice(m_device))
                logcoe::error("Failed to close OpenAL device during shutdown");
            m_device = nullptr;
        }

        m_initialized = false;

        logcoe::info("AudioContext shutdown complete");
        logcoe::shutdown();
    }

    bool AudioContext::isInitialized() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_initialized;
    }

    ALCdevice *AudioContext::getDevice() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_device;
    }

    ALCcontext *AudioContext::getContext() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_context;
    }

    AudioContext &AudioContext::getInstance()
    {
        static std::once_flag initInstanceFlag;

        std::call_once(initInstanceFlag, []() 
        { 
            s_instance.emplace(); 
        });

        return *s_instance;
    }
} // namespace soundcoe