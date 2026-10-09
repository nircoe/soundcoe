#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/core/error_handler.hpp>
#include <soundcoe_config.hpp>
#include <format>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace internal
    {
        namespace
        {
            [[nodiscard]] error alc_failure(ALCdevice *device, const std::string &operation)
            {
                if (auto r = error_handler::check_alc_error(device, operation); !r)
                    return r.error();

                return error_handler::make_error(error_code::alc_error, std::format("{} failed", operation));
            }
        } // namespace

        audio_context::audio_context() { }

        audio_context::~audio_context() { static_cast<void>(shutdown()); }

        std::expected<void, error> audio_context::initialize(const std::string &device_name)
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
                        return {};
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
                    return std::unexpected(alc_failure(nullptr,
                        "Open Audio Device: \"" + (device_name.empty() ? "default" : device_name) + "\""));
            }

            if(!context_created)
            {
                logcoe::info("audio_context::initialize: Initializing audio_context");
                m_context = alcCreateContext(m_device, nullptr);
                if (!m_context)
                {
                    auto err = alc_failure(m_device, "Create Audio Context");
                    alcCloseDevice(m_device);
                    m_device = nullptr;
                    return std::unexpected(err);
                }
            }

            logcoe::debug("audio_context::initialize: Make audio_context current");
            if (!alcMakeContextCurrent(m_context))
            {
                auto err = alc_failure(m_device, "Make Context Current");
                alcDestroyContext(m_context);
                alcCloseDevice(m_device);
                m_device = nullptr;
                m_context = nullptr;
                return std::unexpected(err);
            }

            m_initialized = true;
            logcoe::info("audio_context::initialize: audio_context initialized successfully");
            error_handler::clear_alc_error(m_device);
            return {};
        }

        std::expected<void, error> audio_context::shutdown()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if(!m_initialized)
                return {};

            logcoe::info("audio_context::shutdown: Shutting down audio_context");

            std::expected<void, error> result;

            if (!alcMakeContextCurrent(nullptr))
                result = std::unexpected(alc_failure(m_device, "Make Context Current NULL"));
            else
                logcoe::debug("audio_context::shutdown: Make Context Current NULL succeed");

            if (m_context)
            {
                alcDestroyContext(m_context);
                if (auto r = error_handler::check_alc_error(m_device, "Destroy Context"); !r)
                {
                    if (result)
                        result = std::unexpected(r.error());
                }
                else
                    logcoe::debug("audio_context::shutdown: Destroy Context succeed");
            }
            m_context = nullptr;

            if (m_device)
            {
                if (!alcCloseDevice(m_device))
                {
                    auto err = alc_failure(m_device, "Close Device");
                    if (result)
                        result = std::unexpected(err);
                }
                else
                    logcoe::debug("audio_context::shutdown: Close Device succeed");
            }
            m_device = nullptr;
            m_initialized = false;

            if (result)
                logcoe::info("audio_context::shutdown: audio_context shutdown complete successfully");
            return result;
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
