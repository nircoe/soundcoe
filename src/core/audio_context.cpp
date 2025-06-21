#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <logcoe.hpp>
#include <iostream>

namespace soundcoe
{

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

        logcoe::debug("Initializing ALCdevice: " + (deviceName.empty() ? "default" : deviceName));
        m_device = alcOpenDevice(deviceName.empty() ? nullptr : deviceName.c_str());
        if (!m_device)
        {
            ErrorHandler::checkALCError(nullptr, "Open Audio Device: \"" + deviceName + "\"");
            logcoe::shutdown();
            return false;
        }

        logcoe::info("Initializing AudioContext");
        m_context = alcCreateContext(m_device, nullptr);
        if(!m_context)
        {
            ErrorHandler::checkALCError(m_device, "Create Audio Context");
            alcCloseDevice(m_device);
            m_device = nullptr;
            logcoe::shutdown();
            return false;
        }

        logcoe::debug("Make AudioContext current");
        if(!alcMakeContextCurrent(m_context))
        {
            ErrorHandler::checkALCError(m_device, "Make Context Current");
            alcDestroyContext(m_context);
            alcCloseDevice(m_device);
            m_device = nullptr;
            m_context = nullptr;
            logcoe::shutdown();
            return false;
        }

        m_initialized = true;
        logcoe::info("AudioContext initialized successfully");
        ErrorHandler::clearALCError(m_device);
        return true;
    }

    void AudioContext::shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(!m_initialized)
            return;

        logcoe::info("Shutting down AudioContext");

        if(!alcMakeContextCurrent(nullptr))
            ErrorHandler::checkALCError(m_device, "Make Context Current NULL");
        
        if(m_context)
        {
            alcDestroyContext(m_context);
            ErrorHandler::checkALCError(m_device, "Destroy Context");
            m_context = nullptr;
        }

        if(m_device)
        {
            ErrorHandler::clearALCError(m_device);
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

    AudioContext &AudioContext::getInstance(const std::string &deviceName)
    {
        static AudioContext instance;

        if(!instance.isInitialized())
            if(!instance.initialize(deviceName))
                throw std::runtime_error("Failed to initialize AudioContext");

        return instance;
    }
} // namespace soundcoe