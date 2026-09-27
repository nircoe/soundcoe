#pragma once

#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include <soundcoe/core/types.hpp>

namespace soundcoe
{
    namespace detail
    {
        class error_handler
        {
        public:
            static std::string get_openal_error_as_string(ALenum error);
            static bool check_openal_error(const std::string &operation);
            static void throw_on_openal_error(const std::string &operation);
            static ALenum clear_openal_error();

            static std::string get_alc_error_as_string(ALCenum error);
            static bool check_alc_error(ALCdevice *device, const std::string &operation);
            static void throw_on_alc_error(ALCdevice *device, const std::string &operation);
            static ALCenum clear_alc_error(ALCdevice *device);

            static void throw_on_audio_error(const std::string &filename, audio_format format, audio_decoder_operation operation);
        };
    } // namespace detail
} // namespace soundcoe
