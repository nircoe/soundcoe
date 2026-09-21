# OpenAL-Soft Configuration for soundcoe
# This file contains all OpenAL backend configurations for different platforms

function(fetch_openal_soft)
    # Configure OpenAL: Use Emscripten's built-in OpenAL or fetch OpenAL-Soft
    if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
        message(STATUS "[soundcoe] Using Emscripten's built-in OpenAL implementation")
        # Emscripten provides its own OpenAL implementation via -lopenal linker flag
        # No need to fetch or configure OpenAL-Soft
    else()
        message(STATUS "[soundcoe] Fetching OpenAL-Soft from source...")
        
        FetchContent_Declare(
            openal
            GIT_REPOSITORY https://github.com/kcat/openal-soft.git
            GIT_TAG 1.24.3
            GIT_SHALLOW TRUE
        )
        
        configure_openal()
        FetchContent_MakeAvailable(openal)
        ignore_external_warnings(OpenAL)

        if(MINGW)
            target_link_options(OpenAL PRIVATE -static-libgcc -static-libstdc++ -static)
        endif()
    endif()
endfunction()


# All known OpenAL-Soft backends (null and loopback are always built and have no option)
set(SOUNDCOE_ALL_BACKENDS
    WASAPI DSOUND WINMM                 # Windows
    COREAUDIO                           # macOS/iOS
    OPENSL OBOE                         # Android
    ALSA PULSEAUDIO PIPEWIRE OSS JACK   # Linux
    SOLARIS SNDIO                       # Unix variants
    PORTAUDIO SDL2 SDL3 WAVE OTHERIO    # Cross-platform/utility
)

# Helper function to disable all non-target backends
function(disable_all_backends_except target_backends)
    # Disable all backends not in target list
    foreach(backend ${SOUNDCOE_ALL_BACKENDS})
        list(FIND target_backends ${backend} backend_index)
        if(backend_index EQUAL -1)
            set(ALSOFT_BACKEND_${backend} OFF CACHE BOOL "" FORCE)
        endif()
    endforeach()
endfunction()

# Helper function to disable all require flags
function(disable_all_require_flags)
    set(ALL_REQUIRE_FLAGS
        WASAPI DSOUND WINMM COREAUDIO OPENSL OBOE
        ALSA PULSEAUDIO PIPEWIRE OSS JACK
        PORTAUDIO SOLARIS SNDIO SDL2 SDL3 OTHERIO
    )
    
    foreach(flag ${ALL_REQUIRE_FLAGS})
        set(ALSOFT_REQUIRE_${flag} OFF CACHE BOOL "" FORCE)
    endforeach()
endfunction()

# Helper function to clear backend flags left in the cache by earlier configures
function(reset_backend_flags)
    foreach(backend ${SOUNDCOE_ALL_BACKENDS})
        unset(ALSOFT_BACKEND_${backend} CACHE)
    endforeach()
endfunction()

# Helper function to log available and enabled backends with how to change them
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

# Configure OpenAL-Soft based on target platform
function(configure_openal_for_platform)
    # Reset stale ALSOFT_REQUIRE_* and ALSOFT_BACKEND_* values left in the cache by earlier configures
    disable_all_require_flags()
    reset_backend_flags()

    option(SOUNDCOE_ONLY_NULL_BACKEND
        "Build OpenAL-Soft with only the always-available null backend (headless/CI use)" OFF)

    option(SOUNDCOE_AUTODETECT_BACKENDS
        "Let OpenAL-Soft autodetect backends instead of the platform defaults" OFF)

    if(SOUNDCOE_ONLY_NULL_BACKEND AND SOUNDCOE_AUTODETECT_BACKENDS)
        message(FATAL_ERROR "[soundcoe] SOUNDCOE_ONLY_NULL_BACKEND and SOUNDCOE_AUTODETECT_BACKENDS cannot both be ON")
    endif()

    if(SOUNDCOE_ONLY_NULL_BACKEND)
        message(STATUS "[soundcoe] SOUNDCOE_ONLY_NULL_BACKEND is ON, building null-only headless configuration")
        message(STATUS "[soundcoe] Remove \"set(SOUNDCOE_ONLY_NULL_BACKEND ON)\""
            " to use the platform's real audio backends")
        message(STATUS "[soundcoe] Run with ALSOFT_DRIVERS=null, OpenAL-Soft skips the null backend by default")
        disable_all_backends_except("")
        return()
    endif()

    if(SOUNDCOE_AUTODETECT_BACKENDS)
        message(STATUS "[soundcoe] SOUNDCOE_AUTODETECT_BACKENDS is ON, OpenAL-Soft will pick the backends it finds")
        message(STATUS "[soundcoe] Set SOUNDCOE_AUTODETECT_BACKENDS to OFF to choose backends explicitly,"
            " OpenAL-Soft's own output lists what it found")
        return()
    endif()

    if(WIN32)
        # ================================================================
        # Windows Build
        # ================================================================
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for Windows")
        
        # Windows-specific options
        option(SOUNDCOE_ENABLE_WASAPI "Enable WASAPI backend (Vista+)" ON)
        option(SOUNDCOE_ENABLE_DSOUND "Enable DirectSound backend" ON)
        option(SOUNDCOE_ENABLE_WINMM "Enable WinMM backend" ON)
        option(SOUNDCOE_ENABLE_PORTAUDIO "Enable PortAudio backend" OFF)
        
        # Configure enabled backends
        set(TARGET_BACKENDS "")
        
        if(SOUNDCOE_ENABLE_WASAPI)
            set(ALSOFT_REQUIRE_WASAPI ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "WASAPI")
        endif()
        
        if(SOUNDCOE_ENABLE_DSOUND)
            set(ALSOFT_REQUIRE_DSOUND ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "DSOUND")
        endif()
        
        if(SOUNDCOE_ENABLE_WINMM)
            set(ALSOFT_REQUIRE_WINMM ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "WINMM")
        endif()
        
        if(SOUNDCOE_ENABLE_PORTAUDIO)
            set(ALSOFT_REQUIRE_PORTAUDIO ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "PORTAUDIO")
        endif()
        
        # Disable non-Windows backends
        disable_all_backends_except("${TARGET_BACKENDS}")
        soundcoe_log_backends("Windows" "WASAPI DSOUND WINMM PORTAUDIO" "${TARGET_BACKENDS}" "PORTAUDIO")
        
    elseif(APPLE)
        # ================================================================
        # Apple Build (macOS/iOS)
        # ================================================================
        if(IOS)
            message(STATUS "[soundcoe] Configuring OpenAL-Soft for iOS")
            set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
            set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
            set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
        else()
            message(STATUS "[soundcoe] Configuring OpenAL-Soft for macOS")
        endif()
        
        # Apple platforms use CoreAudio
        set(ALSOFT_REQUIRE_COREAUDIO ON CACHE BOOL "" FORCE)
        disable_all_backends_except("COREAUDIO")
        message(STATUS "[soundcoe] Enabled backends: COREAUDIO (no options on this platform)")
        
    elseif(ANDROID)
        # ================================================================
        # Android Build
        # ================================================================
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for Android")
        
        # Android-specific options
        option(SOUNDCOE_ENABLE_OPENSL "Enable OpenSL ES backend" ON)
        option(SOUNDCOE_ENABLE_OBOE "Enable Oboe backend (newer)" OFF)
        
        set(TARGET_BACKENDS "")
        
        if(SOUNDCOE_ENABLE_OPENSL)
            set(ALSOFT_REQUIRE_OPENSL ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "OPENSL")
        endif()
        
        if(SOUNDCOE_ENABLE_OBOE)
            set(ALSOFT_REQUIRE_OBOE ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "OBOE")
        endif()
        
        disable_all_backends_except("${TARGET_BACKENDS}")
        soundcoe_log_backends("Android" "OPENSL OBOE" "${TARGET_BACKENDS}" "OBOE")
        
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # ================================================================
        # Linux Build  
        # ================================================================
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for Linux")
        
        # Linux-specific options
        option(SOUNDCOE_ENABLE_ALSA "Enable ALSA backend" ON)
        option(SOUNDCOE_ENABLE_PULSEAUDIO "Enable PulseAudio backend" ON)
        option(SOUNDCOE_ENABLE_PIPEWIRE "Enable PipeWire backend" ON)
        option(SOUNDCOE_ENABLE_OSS "Enable OSS backend" OFF)
        option(SOUNDCOE_ENABLE_JACK "Enable JACK backend" OFF)
        option(SOUNDCOE_ENABLE_PORTAUDIO "Enable PortAudio backend" OFF)
        
        set(TARGET_BACKENDS "")
        
        if(SOUNDCOE_ENABLE_ALSA)
            set(ALSOFT_REQUIRE_ALSA ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "ALSA")
        endif()
        
        if(SOUNDCOE_ENABLE_PULSEAUDIO)
            set(ALSOFT_REQUIRE_PULSEAUDIO ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "PULSEAUDIO")
        endif()
        
        if(SOUNDCOE_ENABLE_PIPEWIRE)
            set(ALSOFT_REQUIRE_PIPEWIRE ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "PIPEWIRE")
        endif()
        
        if(SOUNDCOE_ENABLE_OSS)
            set(ALSOFT_REQUIRE_OSS ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "OSS")
        endif()
        
        if(SOUNDCOE_ENABLE_JACK)
            set(ALSOFT_REQUIRE_JACK ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "JACK")
        endif()
        
        if(SOUNDCOE_ENABLE_PORTAUDIO)
            set(ALSOFT_REQUIRE_PORTAUDIO ON CACHE BOOL "" FORCE)
            list(APPEND TARGET_BACKENDS "PORTAUDIO")
        endif()
        
        disable_all_backends_except("${TARGET_BACKENDS}")
        soundcoe_log_backends("Linux" "ALSA PULSEAUDIO PIPEWIRE OSS JACK PORTAUDIO" "${TARGET_BACKENDS}" "JACK")

    elseif(UNIX)
        # ================================================================
        # Other UNIX Build (FreeBSD etc.)
        # ================================================================
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for ${CMAKE_SYSTEM_NAME} (backend autodetection)")
        message(STATUS "[soundcoe] No backend options on this platform,"
            " see OpenAL-Soft's own output for the backends it found")
        # No force-disable and no REQUIRE flags so OpenAL-Soft autodetects (sndio, OSS, Solaris, ...)

    else()
        # ================================================================
        # Unknown Platform - Conservative Configuration
        # ================================================================
        message(WARNING "[soundcoe] Unknown platform: ${CMAKE_SYSTEM_NAME}")
        message(STATUS "[soundcoe] Using conservative OpenAL-Soft configuration")
        
        # Disable all optional backends for safe fallback (null is always built)
        disable_all_backends_except("")

    endif()
endfunction()

function(configure_openal)
    # Global OpenAL-Soft settings (applies to all platforms)
    set(LIBTYPE "SHARED" CACHE STRING "Build shared library" FORCE)
    set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
    set(ALSOFT_EMBED_HRTF_DATA ON CACHE BOOL "" FORCE)
    
    # Platform-specific backend configuration
    configure_openal_for_platform()
endfunction()