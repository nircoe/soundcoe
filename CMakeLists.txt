cmake_minimum_required(VERSION 3.22)
project(soundcoe LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(SOUNDCOE_SOURCE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(FetchContent)
include(cmake/soundcoe_config.cmake)

find_package(Threads REQUIRED)

fetch_openal_soft()
fetch_logcoe()

generate_soundcoe_config_header()

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
