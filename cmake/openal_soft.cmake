function(soundcoe_log_backends platform available enabled example)
    string(REPLACE ";" " " enabled_text "${enabled}")
    if(enabled_text STREQUAL "")
        set(enabled_text "none")
    endif()
    message(STATUS "[soundcoe] Available backends (${platform}): ${available}")
    message(STATUS "[soundcoe] Enabled backends: ${enabled_text}")
    message(STATUS "[soundcoe] Can be changed with \"set(SOUNDCOE_ENABLE_<BACKEND> ON|OFF)\" before adding soundcoe")
    message(STATUS "[soundcoe] For example: \"set(SOUNDCOE_ENABLE_${example} ON)\"")
    message(STATUS
        "[soundcoe] Other modes: \"set(SOUNDCOE_AUTODETECT_BACKENDS ON)\" (OpenAL-Soft picks what it finds)")
    message(STATUS "[soundcoe] and \"set(SOUNDCOE_ONLY_NULL_BACKEND ON)\" (no audio output, headless/CI use)")
endfunction()

function(soundcoe_disable_all_backends_except target_backends)
    set(all_backends
        WASAPI DSOUND WINMM COREAUDIO OPENSL OBOE ALSA PULSEAUDIO PIPEWIRE OSS JACK
        SOLARIS SNDIO PORTAUDIO SDL2 SDL3 WAVE OTHERIO
    )
    foreach(backend ${all_backends})
        if(NOT backend IN_LIST target_backends)
            set(ALSOFT_BACKEND_${backend} OFF PARENT_SCOPE)
        endif()
    endforeach()
endfunction()

function(soundcoe_fetch_openal_soft)
    if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
        message(STATUS "[soundcoe] Using Emscripten's built-in OpenAL implementation")
        return()
    endif()

    if(TARGET OpenAL)
        message(STATUS "[soundcoe] OpenAL already available, skipping fetch")
        return()
    endif()

    option(SOUNDCOE_ONLY_NULL_BACKEND
        "Build OpenAL-Soft with only the always-available null backend (headless/CI use)" OFF)

    option(SOUNDCOE_AUTODETECT_BACKENDS
        "Let OpenAL-Soft autodetect backends instead of the platform defaults" OFF)

    if(SOUNDCOE_ONLY_NULL_BACKEND AND SOUNDCOE_AUTODETECT_BACKENDS)
        message(FATAL_ERROR "[soundcoe] SOUNDCOE_ONLY_NULL_BACKEND and SOUNDCOE_AUTODETECT_BACKENDS cannot both be ON")
    endif()

    # OpenAL-Soft's option() honors these plain variables (CMP0077), but only when they are set
    # in the same function that calls FetchContent_MakeAvailable
    set(LIBTYPE SHARED)
    set(ALSOFT_UTILS OFF)
    set(ALSOFT_EXAMPLES OFF)
    set(ALSOFT_TESTS OFF)
    set(ALSOFT_NO_CONFIG_UTIL ON)
    set(ALSOFT_EMBED_HRTF_DATA ON)

    if(MINGW)
        set(ALSOFT_STATIC_LIBGCC ON)
        set(ALSOFT_STATIC_STDCXX ON)
        set(ALSOFT_STATIC_WINPTHREAD ON)
    endif()

    if(SOUNDCOE_ONLY_NULL_BACKEND)
        message(STATUS "[soundcoe] SOUNDCOE_ONLY_NULL_BACKEND is ON, building null-only headless configuration")
        message(STATUS "[soundcoe] Remove \"set(SOUNDCOE_ONLY_NULL_BACKEND ON)\""
            " to use the platform's real audio backends")
        message(STATUS "[soundcoe] Run with ALSOFT_DRIVERS=null, OpenAL-Soft skips the null backend by default")
        soundcoe_disable_all_backends_except("")
    elseif(SOUNDCOE_AUTODETECT_BACKENDS)
        message(STATUS "[soundcoe] SOUNDCOE_AUTODETECT_BACKENDS is ON, OpenAL-Soft will pick the backends it finds")
        message(STATUS "[soundcoe] Set SOUNDCOE_AUTODETECT_BACKENDS to OFF to choose backends explicitly,"
            " OpenAL-Soft's own output lists what it found")
    elseif(WIN32)
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for Windows")

        option(SOUNDCOE_ENABLE_WASAPI "Enable WASAPI backend (Vista+)" ON)
        option(SOUNDCOE_ENABLE_DSOUND "Enable DirectSound backend" ON)
        option(SOUNDCOE_ENABLE_WINMM "Enable WinMM backend" ON)
        option(SOUNDCOE_ENABLE_PORTAUDIO "Enable PortAudio backend" OFF)

        set(target_backends "")

        if(SOUNDCOE_ENABLE_WASAPI)
            set(ALSOFT_REQUIRE_WASAPI ON)
            list(APPEND target_backends "WASAPI")
        endif()

        if(SOUNDCOE_ENABLE_DSOUND)
            set(ALSOFT_REQUIRE_DSOUND ON)
            list(APPEND target_backends "DSOUND")
        endif()

        if(SOUNDCOE_ENABLE_WINMM)
            set(ALSOFT_REQUIRE_WINMM ON)
            list(APPEND target_backends "WINMM")
        endif()

        if(SOUNDCOE_ENABLE_PORTAUDIO)
            set(ALSOFT_REQUIRE_PORTAUDIO ON)
            list(APPEND target_backends "PORTAUDIO")
        endif()

        soundcoe_disable_all_backends_except("${target_backends}")
        soundcoe_log_backends("Windows" "WASAPI DSOUND WINMM PORTAUDIO" "${target_backends}" "PORTAUDIO")
    elseif(APPLE)
        if(IOS)
            message(STATUS "[soundcoe] Configuring OpenAL-Soft for iOS")
            set(ALSOFT_INSTALL OFF)
        else()
            message(STATUS "[soundcoe] Configuring OpenAL-Soft for macOS")
        endif()

        set(ALSOFT_REQUIRE_COREAUDIO ON)
        soundcoe_disable_all_backends_except("COREAUDIO")
        message(STATUS "[soundcoe] Enabled backends: COREAUDIO (no options on this platform)")
    elseif(ANDROID)
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for Android")

        option(SOUNDCOE_ENABLE_OPENSL "Enable OpenSL ES backend" ON)
        option(SOUNDCOE_ENABLE_OBOE "Enable Oboe backend (newer)" OFF)

        set(target_backends "")

        if(SOUNDCOE_ENABLE_OPENSL)
            set(ALSOFT_REQUIRE_OPENSL ON)
            list(APPEND target_backends "OPENSL")
        endif()

        if(SOUNDCOE_ENABLE_OBOE)
            set(ALSOFT_REQUIRE_OBOE ON)
            list(APPEND target_backends "OBOE")
        endif()

        soundcoe_disable_all_backends_except("${target_backends}")
        soundcoe_log_backends("Android" "OPENSL OBOE" "${target_backends}" "OBOE")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for Linux")

        option(SOUNDCOE_ENABLE_ALSA "Enable ALSA backend" ON)
        option(SOUNDCOE_ENABLE_PULSEAUDIO "Enable PulseAudio backend" ON)
        option(SOUNDCOE_ENABLE_PIPEWIRE "Enable PipeWire backend" ON)
        option(SOUNDCOE_ENABLE_OSS "Enable OSS backend" OFF)
        option(SOUNDCOE_ENABLE_JACK "Enable JACK backend" OFF)
        option(SOUNDCOE_ENABLE_PORTAUDIO "Enable PortAudio backend" OFF)

        set(target_backends "")

        if(SOUNDCOE_ENABLE_ALSA)
            set(ALSOFT_REQUIRE_ALSA ON)
            list(APPEND target_backends "ALSA")
        endif()

        if(SOUNDCOE_ENABLE_PULSEAUDIO)
            set(ALSOFT_REQUIRE_PULSEAUDIO ON)
            list(APPEND target_backends "PULSEAUDIO")
        endif()

        if(SOUNDCOE_ENABLE_PIPEWIRE)
            set(ALSOFT_REQUIRE_PIPEWIRE ON)
            list(APPEND target_backends "PIPEWIRE")
        endif()

        if(SOUNDCOE_ENABLE_OSS)
            set(ALSOFT_REQUIRE_OSS ON)
            list(APPEND target_backends "OSS")
        endif()

        if(SOUNDCOE_ENABLE_JACK)
            set(ALSOFT_REQUIRE_JACK ON)
            list(APPEND target_backends "JACK")
        endif()

        if(SOUNDCOE_ENABLE_PORTAUDIO)
            set(ALSOFT_REQUIRE_PORTAUDIO ON)
            list(APPEND target_backends "PORTAUDIO")
        endif()

        soundcoe_disable_all_backends_except("${target_backends}")
        soundcoe_log_backends("Linux" "ALSA PULSEAUDIO PIPEWIRE OSS JACK PORTAUDIO" "${target_backends}" "JACK")
    elseif(UNIX)
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for ${CMAKE_SYSTEM_NAME} (backend autodetection)")
        message(STATUS "[soundcoe] No backend options on this platform,"
            " see OpenAL-Soft's own output for the backends it found")
        # No REQUIRE flags and no disabled backends, so OpenAL-Soft autodetects (sndio, OSS, Solaris, ...)
    else()
        message(WARNING "[soundcoe] Unknown platform: ${CMAKE_SYSTEM_NAME}")
        message(STATUS "[soundcoe] Using conservative OpenAL-Soft configuration")
        soundcoe_disable_all_backends_except("")
    endif()

    message(STATUS "[soundcoe] Fetching OpenAL-Soft from source...")

    FetchContent_Declare(
        openal
        GIT_REPOSITORY https://github.com/kcat/openal-soft.git
        GIT_TAG 1.24.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(openal)
    soundcoe_ignore_external_warnings(OpenAL)
endfunction()
