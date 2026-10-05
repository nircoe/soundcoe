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
        std::string create_error_message(const std::string& error_type, const std::string& operation,
            const std::string& details)
        {
            return std::format("{}{} - {}", error_type, operation, details);
        }

        namespace error_handler
        {
            error make_error(error_code code, const std::string &message)
            {
                logcoe::error(message);
                return error{code, message};
            }

            std::string get_openal_error_as_string(ALenum al_error)
            {
                switch(al_error)
                {
                    case AL_NO_ERROR:
                        return "AL_NO_ERROR";
                    case AL_INVALID_NAME:
                        return "AL_INVALID_NAME";
                    case AL_INVALID_ENUM:
                        return "AL_INVALID_ENUM";
                    case AL_INVALID_VALUE:
                        return "AL_INVALID_VALUE";
                    case AL_INVALID_OPERATION:
                        return "AL_INVALID_OPERATION";
                    case AL_OUT_OF_MEMORY:
                        return "AL_OUT_OF_MEMORY";
                    default:
                        return "UNKNOWN ERROR";
                }
            }

            std::expected<void, error> check_openal_error(const std::string &operation)
            {
                ALenum al_error = alGetError();
                if(al_error == AL_NO_ERROR)
                    return {};

                return std::unexpected(make_error(error_code::openal_error,
                    create_error_message("OpenAL Error: ", operation, get_openal_error_as_string(al_error))));
            }

            ALenum clear_openal_error()
            {
                return alGetError();
            }

            std::string get_alc_error_as_string(ALCenum alc_error)
            {
                switch(alc_error)
                {
                    case ALC_NO_ERROR:
                        return "ALC_NO_ERROR";
                    case ALC_INVALID_DEVICE:
                        return "ALC_INVALID_DEVICE";
                    case ALC_INVALID_CONTEXT:
                        return "ALC_INVALID_CONTEXT";
                    case ALC_INVALID_ENUM:
                        return "ALC_INVALID_ENUM";
                    case ALC_INVALID_VALUE:
                        return "ALC_INVALID_VALUE";
                    case ALC_OUT_OF_MEMORY:
                        return "ALC_OUT_OF_MEMORY";
                    default:
                        return "UNKNOWN ERROR";
                }
            }

            std::expected<void, error> check_alc_error(ALCdevice *device, const std::string &operation)
            {
                ALCenum alc_error = alcGetError(device);
                if(alc_error == ALC_NO_ERROR)
                    return {};

                return std::unexpected(make_error(error_code::alc_error,
                    create_error_message("ALC Error: ", operation, get_alc_error_as_string(alc_error))));
            }

            ALCenum clear_alc_error(ALCdevice *device)
            {
                return alcGetError(device);
            }

            error make_audio_decode_error(const std::string &filename, audio_format format,
                                          audio_decoder_operation operation)
            {
                return make_error(error_code::audio_decode_failure,
                    std::format("Audio Decoder Error: {} - {} - {}", filename, to_string(format),
                        to_string(operation)));
            }
        } // namespace error_handler
    } // namespace internal
} // namespace soundcoe
