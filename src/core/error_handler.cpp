#include <soundcoe/core/error_handler.hpp>
#include <iostream>
#include <sstream>
#include <exception>
#include <soundcoe_config.hpp>
#if SOUNDCOE_USE_LOGCOE
#include <logcoe.hpp>
#endif

namespace soundcoe
{
    namespace internal
    {
        std::string create_error_message(const std::string& error_type, const std::string& operation, const std::string& error)
        {
            std::stringstream message;
            message << error_type << operation << " - " << error;
            return message.str();
        }

        std::string error_handler::get_openal_error_as_string(ALenum error)
        {
            switch(error)
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

        bool error_handler::check_openal_error(const std::string &operation)
        {
            ALenum error = alGetError();
            if(error == AL_NO_ERROR)
                return false;

            std::string message = create_error_message("OpenAL Error: ", operation, get_openal_error_as_string(error));
            logcoe::error(message);
            return true;
        }

        void error_handler::throw_on_openal_error(const std::string &operation)
        {
            ALenum error = alGetError();
            if (error == AL_NO_ERROR) return;

            std::string message = create_error_message("OpenAL Error: ", operation, get_openal_error_as_string(error));
            logcoe::error(message);
            throw std::runtime_error(message);
        }

        ALenum error_handler::clear_openal_error()
        {
            return alGetError();
        }

        std::string error_handler::get_alc_error_as_string(ALCenum error)
        {
            switch(error)
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

        bool error_handler::check_alc_error(ALCdevice *device, const std::string &operation)
        {
            ALCenum error = alcGetError(device);
            if(error == ALC_NO_ERROR)
                return false;

            std::string message = create_error_message("ALC Error: ", operation, get_alc_error_as_string(error));
            logcoe::error(message);
            return true;
        }

        void error_handler::throw_on_alc_error(ALCdevice *device, const std::string &operation)
        {
            ALCenum error = alcGetError(device);
            if (error == ALC_NO_ERROR) return;

            std::string message = create_error_message("ALC Error: ", operation, get_alc_error_as_string(error));
            logcoe::error(message);
            throw std::runtime_error(message);
        }

        ALCenum error_handler::clear_alc_error(ALCdevice *device)
        {
            return alcGetError(device);
        }

        void error_handler::throw_on_audio_error(const std::string &filename, audio_format format, audio_decoder_operation operation)
        {
            std::ostringstream oss;
            oss << "Audio Decoder Error: " << filename << " - " << to_string(format) << " - " << to_string(operation);
            std::string message = oss.str();
            logcoe::error(message);
            throw std::runtime_error(message);
        }
    } // namespace internal
} // namespace soundcoe
