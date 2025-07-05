cmake_minimum_required(VERSION 3.14)

include(FetchContent)
FetchContent_Declare(
    testcoe
    GIT_REPOSITORY https://github.com/nircoe/testcoe.git
    GIT_TAG v0.1.0
)


FetchContent_MakeAvailable(testcoe)

add_executable(soundcoe_tests
    main.cpp
    core_tests.cpp
)

target_link_libraries(soundcoe_tests
    PRIVATE
        soundcoe
        testcoe
        OpenAL
)