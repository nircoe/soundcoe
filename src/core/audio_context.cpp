#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <logcoe.hpp>
#include <iostream>

namespace soundcoe
{

    AudioContext::AudioContext() { }

    AudioContext::~AudioContext() { shutdown(); }

    void AudioContext::initialize(const std::string &deviceName)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_initialized)
        {
            logcoe::info("AudioContext is already initialized");
            return;
        }

        logcoe::debug("Initializing ALCdevice: " + (deviceName.empty() ? "default" : deviceName));
        m_device = alcOpenDevice(deviceName.empty() ? nullptr : deviceName.c_str());
        if (!m_device)
            ErrorHandler::throwOnALCError(nullptr, "Open Audio Device: \"" + (deviceName.empty() ? "default" : deviceName) + "\"");

        logcoe::info("Initializing AudioContext");
        m_context = alcCreateContext(m_device, nullptr);
        if(!m_context)
        {
            try { ErrorHandler::throwOnALCError(m_device, "Create Audio Context"); }
            catch(const std::runtime_error &e)
            {
                alcCloseDevice(m_device);
                m_device = nullptr;
                throw;
            }
        }

        logcoe::debug("Make AudioContext current");
        if(!alcMakeContextCurrent(m_context))
        {
            try { ErrorHandler::throwOnALCError(m_device, "Make Context Current"); }
            catch(const std::runtime_error &e)
            {
                alcDestroyContext(m_context);
                alcCloseDevice(m_device);
                m_device = nullptr;
                m_context = nullptr;
                return;
            }
        }

        m_initialized = true;
        logcoe::info("AudioContext initialized successfully");
        ErrorHandler::clearALCError(m_device);
    }

    void AudioContext::shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(!m_initialized)
            return;

        logcoe::info("Shutting down AudioContext");

        if(!alcMakeContextCurrent(nullptr))
            ErrorHandler::throwOnALCError(m_device, "Make Context Current NULL");
        logcoe::debug("Make Context Current NULL succeed");

        if(m_context)
        {
            alcDestroyContext(m_context);
            ErrorHandler::throwOnALCError(m_device, "Destroy Context");
        }
        logcoe::debug("Destroy Context succeed");
        m_context = nullptr;

        if(m_device && !alcCloseDevice(m_device))
            ErrorHandler::throwOnALCError(m_device, "Close Device");
        logcoe::debug("Close Device succeed");
        m_device = nullptr;
        m_initialized = false;
        logcoe::info("AudioContext shutdown complete successfully");
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
} // namespace soundcoe