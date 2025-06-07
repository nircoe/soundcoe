cmake_minimum_required(VERSION 3.14)
project(soundcoe VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "[soundcoe] Fetching OpenAL-Soft from source...")

include(FetchContent)
FetchContent_Declare(
    openal
    GIT_REPOSITORY https://github.com/kcat/openal-soft.git
    GIT_TAG 1.24.3
)

# Basic configuration options (for all platforms)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
set(ALSOFT_EMBED_HRTF_DATA ON CACHE BOOL "" FORCE)

# Platform-specific backend requirements
if(WIN32)
  # Windows
  set(ALSOFT_REQUIRE_WINMM ON CACHE BOOL "" FORCE)
  set(ALSOFT_REQUIRE_DSOUND ON CACHE BOOL "" FORCE)
  set(ALSOFT_REQUIRE_WASAPI ON CACHE BOOL "" FORCE)
elseif(APPLE)
  if(IOS)
    # iOS
    set(ALSOFT_REQUIRE_COREAUDIO ON CACHE BOOL "" FORCE)
    set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
  else()
    # macOS
    set(ALSOFT_REQUIRE_COREAUDIO ON CACHE BOOL "" FORCE)
  endif()
elseif(ANDROID)
  # Android
  set(ALSOFT_REQUIRE_OPENSL ON CACHE BOOL "" FORCE)
elseif(UNIX)
  # Linux
  set(ALSOFT_REQUIRE_ALSA ON CACHE BOOL "" FORCE)
  set(ALSOFT_REQUIRE_PULSEAUDIO ON CACHE BOOL "" FORCE)
  
  # PipeWire is becoming the standard on modern Linux distros
  option(SOUNDCOE_ENABLE_PIPEWIRE "Enable PipeWire backend if available" ON)
  if(SOUNDCOE_ENABLE_PIPEWIRE)
    set(ALSOFT_REQUIRE_PIPEWIRE ON CACHE BOOL "" FORCE)
  endif()
endif()

FetchContent_MakeAvailable(openal)

message(STATUS "[soundcoe] Fetching logcoe from source...")

FetchContent_Declare(
    logcoe
    GIT_REPOSITORY https://github.com/nircoe/logcoe.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(logcoe)

add_subdirectory(include)
add_subdirectory(src)

install(TARGETS soundcoe soundcoe_headers
    EXPORT soundcoe-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(EXPORT soundcoe-targets
    FILE soundcoe-targets.cmake
    DESTINATION lib/cmake/soundcoe
)