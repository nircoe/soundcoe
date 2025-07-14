cmake_minimum_required(VERSION 3.14)

message(STATUS "[soundcoe] Fetching testcoe from source...")
include(FetchContent)
FetchContent_Declare(
    testcoe
    GIT_REPOSITORY https://github.com/nircoe/testcoe.git
    GIT_TAG v0.1.0
)

FetchContent_MakeAvailable(testcoe)
ignore_external_warnings(testcoe)

add_executable(soundcoe_tests
    main.cpp
    core_tests.cpp
    resources_tests.cpp
    math_tests.cpp
    playback_tests.cpp
)

target_link_libraries(soundcoe_tests
    PRIVATE
        soundcoe
        testcoe
        OpenAL
)