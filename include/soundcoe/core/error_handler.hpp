#pragma once

#include <soundcoe/core/error.hpp>
#include <soundcoe/core/types.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <expected>
#include <string>
#include <system_error>

namespace soundcoe
{
    namespace internal
    {
        namespace error_handler
        {
            [[nodiscard]] error make_error(error_code code, const std::string &message);

            std::string get_openal_error_as_string(ALenum al_error);
            [[nodiscard]] std::expected<void, error> check_openal_error(const std::string &operation);
            ALenum clear_openal_error();

            std::string get_alc_error_as_string(ALCenum alc_error);
            [[nodiscard]] std::expected<void, error> check_alc_error(ALCdevice *device,
                                                                       const std::string &operation);
            ALCenum clear_alc_error(ALCdevice *device);

            [[nodiscard]] error make_audio_decode_error(const std::string &filename, audio_format format,
                                                          audio_decoder_operation operation);
            [[nodiscard]] error make_filesystem_error(const std::string &method, const std::string &path,
                                                        std::error_code ec);
        } // namespace error_handler
    } // namespace internal
} // namespace soundcoe
