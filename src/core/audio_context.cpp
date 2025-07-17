#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <logcoe.hpp>
#include <iostream>

namespace soundcoe
{
    namespace detail
    {
        AudioContext::AudioContext() { }

        AudioContext::~AudioContext() { shutdown(); }

        void AudioContext::initialize(const std::string &deviceName)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            bool deviceOpened = false, contextCreated = false;
            if(m_initialized)
            {
                ALCcontext *current = alcGetCurrentContext();
                if (m_device && m_context)
                {
                    if(m_context == current)
                    {
                        logcoe::info("AudioContext is already initialized");
                        return;
                    }
                    deviceOpened = true;
                    contextCreated = true;
                }
                else if(m_device && !m_context)
                {
                    deviceOpened = true;
                    contextCreated = false;
                }
            }

            if(!deviceOpened)
            {
                logcoe::debug("Initializing ALCdevice: " + (deviceName.empty() ? "default" : deviceName));
                m_device = alcOpenDevice(deviceName.empty() ? nullptr : deviceName.c_str());
                if (!m_device)
                    ErrorHandler::throwOnALCError(nullptr, "Open Audio Device: \"" + (deviceName.empty() ? "default" : deviceName) + "\"");
            }

            if(!contextCreated)
            {
                logcoe::info("Initializing AudioContext");
                m_context = alcCreateContext(m_device, nullptr);
                if(!m_context)
                {
                    try { ErrorHandler::throwOnALCError(m_device, "Create Audio Context"); }
                    catch(const std::runtime_error &)
                    {
                        alcCloseDevice(m_device);
                        m_device = nullptr;
                        throw;
                    }
                }
            }

            logcoe::debug("Make AudioContext current");
            if(!alcMakeContextCurrent(m_context))
            {
                try { ErrorHandler::throwOnALCError(m_device, "Make Context Current"); }
                catch(const std::runtime_error &)
                {
                    alcDestroyContext(m_context);
                    alcCloseDevice(m_device);
                    m_device = nullptr;
                    m_context = nullptr;
                    throw;
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
    } // namespace detail
} // namespace soundcoe