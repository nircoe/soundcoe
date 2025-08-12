cmake_minimum_required(VERSION 3.14)
project(soundcoe VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(cmake/soundcoe_config.cmake)

find_package(Threads REQUIRED)

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
    )
    
    configure_openal()
    FetchContent_MakeAvailable(openal)
    ignore_external_warnings(OpenAL)
endif()

message(STATUS "[soundcoe] Fetching logcoe from source...")

FetchContent_Declare(
    logcoe
    GIT_REPOSITORY https://github.com/nircoe/logcoe.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(logcoe)
ignore_external_warnings(logcoe)

add_subdirectory(include)
add_subdirectory(src)

option(SOUNDCOE_BUILD_TESTS "Build soundcoe tests" OFF)
if(SOUNDCOE_BUILD_TESTS)
  message(STATUS "[soundcoe] Building soundcoe tests")
  add_subdirectory(tests)
endif()

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