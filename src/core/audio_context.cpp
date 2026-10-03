#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <stdexcept>
#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace internal
    {
        audio_context::audio_context() { }

        audio_context::~audio_context() { shutdown(); }

        void audio_context::initialize(const std::string &device_name)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            bool device_opened = false, context_created = false;
            if(m_initialized)
            {
                ALCcontext *current = alcGetCurrentContext();
                if (m_device && m_context)
                {
                    if(m_context == current)
                    {
                        logcoe::info("audio_context::initialize: audio_context is already initialized");
                        return;
                    }
                    device_opened = true;
                    context_created = true;
                }
                else if(m_device && !m_context)
                {
                    device_opened = true;
                    context_created = false;
                }
            }

            if(!device_opened)
            {
                logcoe::debug("audio_context::initialize: Initializing ALCdevice: " + (device_name.empty() ? "default" : device_name));
                m_device = alcOpenDevice(device_name.empty() ? nullptr : device_name.c_str());
                if (!m_device)
                    error_handler::throw_on_alc_error(nullptr, "Open Audio Device: \"" + (device_name.empty() ? "default" : device_name) + "\"");
            }

            if(!context_created)
            {
                logcoe::info("audio_context::initialize: Initializing audio_context");
                m_context = alcCreateContext(m_device, nullptr);
                if(!m_context)
                {
                    try { error_handler::throw_on_alc_error(m_device, "Create Audio Context"); }
                    catch(const std::runtime_error &)
                    {
                        alcCloseDevice(m_device);
                        m_device = nullptr;
                        throw;
                    }
                }
            }

            logcoe::debug("audio_context::initialize: Make audio_context current");
            if(!alcMakeContextCurrent(m_context))
            {
                try { error_handler::throw_on_alc_error(m_device, "Make Context Current"); }
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
            logcoe::info("audio_context::initialize: audio_context initialized successfully");
            error_handler::clear_alc_error(m_device);
        }

        void audio_context::shutdown()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if(!m_initialized)
                return;

            logcoe::info("audio_context::shutdown: Shutting down audio_context");

            if(!alcMakeContextCurrent(nullptr))
                error_handler::throw_on_alc_error(m_device, "Make Context Current NULL");
            logcoe::debug("audio_context::shutdown: Make Context Current NULL succeed");

            if(m_context)
            {
                alcDestroyContext(m_context);
                error_handler::throw_on_alc_error(m_device, "Destroy Context");
            }
            logcoe::debug("audio_context::shutdown: Destroy Context succeed");
            m_context = nullptr;

            if(m_device && !alcCloseDevice(m_device))
                error_handler::throw_on_alc_error(m_device, "Close Device");
            logcoe::debug("audio_context::shutdown: Close Device succeed");
            m_device = nullptr;
            m_initialized = false;
            logcoe::info("audio_context::shutdown: audio_context shutdown complete successfully");
        }

        bool audio_context::is_initialized() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_initialized;
        }

        ALCdevice *audio_context::get_device() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_device;
        }

        ALCcontext *audio_context::get_context() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_context;
        }
    } // namespace internal
} // namespace soundcoe
