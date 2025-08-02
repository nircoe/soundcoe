# OpenAL-Soft Configuration for soundcoe
# This file contains all OpenAL backend configurations for different platforms

# Helper function to disable all non-target backends
function(disable_all_backends_except target_backends)
    # All known OpenAL-Soft backends
    set(ALL_BACKENDS
        WASAPI DSOUND WINMM           # Windows
        COREAUDIO                     # macOS/iOS  
        OPENSL OBOE                   # Android
        ALSA PULSEAUDIO PIPEWIRE OSS JACK  # Linux
        SOLARIS SNDIO                 # Unix variants
        PORTAUDIO SDL2 SDL3 WAVE NULL # Cross-platform/utility
    )
    
    # Disable all backends not in target list
    foreach(backend ${ALL_BACKENDS})
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
        PORTAUDIO SOLARIS SNDIO SDL2 SDL3
    )
    
    foreach(flag ${ALL_REQUIRE_FLAGS})
        set(ALSOFT_REQUIRE_${flag} OFF CACHE BOOL "" FORCE)
    endforeach()
endfunction()

# Configure OpenAL-Soft based on target platform
function(configure_openal_for_platform)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
        # ================================================================
        # Emscripten/Web Build - Use Emscripten's built-in OpenAL
        # ================================================================
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for Emscripten/Web build")
        
        # Disable ALL native backends except NULL backend
        disable_all_backends_except("NULL")
        disable_all_require_flags()
        
    elseif(WIN32)
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
        
    elseif(UNIX)
        # ================================================================
        # Linux/Unix Build  
        # ================================================================
        message(STATUS "[soundcoe] Configuring OpenAL-Soft for Linux/Unix")
        
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
        
    else()
        # ================================================================
        # Unknown Platform - Conservative Configuration
        # ================================================================
        message(WARNING "[soundcoe] Unknown platform: ${CMAKE_SYSTEM_NAME}")
        message(STATUS "[soundcoe] Using conservative OpenAL-Soft configuration")
        
        # Disable all backends except NULL for safe fallback
        disable_all_backends_except("NULL")
        disable_all_require_flags()
        
    endif()
endfunction()

function(configure_openal)
    # Global OpenAL-Soft settings (applies to all platforms)
    set(LIBTYPE "STATIC" CACHE STRING "Build static library" FORCE)
    set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
    set(ALSOFT_EMBED_HRTF_DATA ON CACHE BOOL "" FORCE)
    
    # Platform-specific backend configuration
    configure_openal_for_platform()
endfunction()