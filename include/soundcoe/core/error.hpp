#pragma once

#include <string>

namespace soundcoe
{
    enum class error_code
    {
        alc_error,
        already_initialized,
        audio_decode_failure,
        directory_not_found,
        file_not_found,
        filesystem_error,
        invalid_argument,
        invalid_handle,
        invalid_state,
        not_initialized,
        openal_error,
        resource_exhausted,
        source_inactive,
        unsupported_format
    };

    struct error
    {
        error_code code;
        std::string message;
    };
} // namespace soundcoe
