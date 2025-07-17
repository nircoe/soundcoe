#pragma once

#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include <soundcoe/core/types.hpp>

namespace soundcoe
{
    namespace detail
    {
        class ErrorHandler
        {
        public:
            static std::string getOpenALErrorAsString(ALenum error);
            static bool checkOpenALError(const std::string &operation);
            static void throwOnOpenALError(const std::string &operation);
            static ALenum clearOpenALError();

            static std::string getALCErrorAsString(ALCenum error);
            static bool checkALCError(ALCdevice *device, const std::string &operation);
            static void throwOnALCError(ALCdevice *device, const std::string &operation);
            static ALCenum clearALCError(ALCdevice *device);

            static void throwOnAudioError(const std::string &filename, AudioFormat format, AudioDecoderOperation operation);
        };
    } // namespace detail
} // namespace soundcoe