cmake_minimum_required(VERSION 3.22)
project(soundcoe VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(SOUNDCOE_SOURCE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(FetchContent)
include(cmake/soundcoe_config.cmake)

option(SOUNDCOE_BUILD_TESTS "Build the soundcoe test suite" OFF)

find_package(Threads REQUIRED)

fetch_openal_soft()
soundcoe_fetch_logcoe()
if(SOUNDCOE_BUILD_TESTS)
    soundcoe_fetch_testcoe()
endif()

soundcoe_generate_config_header()

add_subdirectory(external)
add_subdirectory(include)
add_subdirectory(src)

if(SOUNDCOE_BUILD_TESTS)
    message(STATUS "[soundcoe] Building soundcoe tests")
    add_subdirectory(tests)
endif()
