#include <soundcoe/core/error_handler.hpp>
#include <logcoe.hpp>
#include <iostream>
#include <sstream>

namespace soundcoe
{
    std::string ErrorHandler::getOpenALErrorAsString(ALenum error)
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

    bool ErrorHandler::checkOpenALError(const std::string &operation)
    {
        ALenum error = alGetError();
        if(error == AL_NO_ERROR)
            return false;

        std::stringstream ss;
        ss << "OpenAL Error: " << operation << " - " << getOpenALErrorAsString(error);
        logcoe::error(ss.str());
        return true;
    }

    ALenum ErrorHandler::clearOpenALError()
    {
        return alGetError();
    }

    std::string ErrorHandler::getALCErrorAsString(ALCenum error)
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

    bool ErrorHandler::checkALCError(ALCdevice *device, const std::string &operation)
    {
        ALCenum error = alcGetError(device);
        if(error == ALC_NO_ERROR)
            return false;

        std::stringstream ss;
        ss << "ALC Error: " << operation << " - " << getALCErrorAsString(error);
        logcoe::error(ss.str());
        return true;
    }

    ALCenum ErrorHandler::clearALCError(ALCdevice *device)
    {
        return alcGetError(device);
    }
} // namespace soundcoe