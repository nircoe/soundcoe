#pragma once

#include <string>
#include <AL/al.h>
#include <AL/alc.h>

namespace soundcoe
{
    class ErrorHandler
    {
    public:
        static std::string getOpenALErrorAsString(ALenum error);
        static bool checkOpenALError(const std::string &operation);
        static ALenum clearOpenALError();

        static std::string getALCErrorAsString(ALCenum error);
        static bool checkALCError(ALCdevice *device, const std::string &operation);
        static ALCenum clearALCError(ALCdevice *device);
    };
} // namespace soundcoe