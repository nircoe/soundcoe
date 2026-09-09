message(STATUS "[soundcoe] Fetching testcoe from source...")
include(FetchContent)
FetchContent_Declare(
    testcoe
    GIT_REPOSITORY https://github.com/nircoe/testcoe.git
    GIT_TAG v0.1.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(testcoe)

ignore_external_warnings(gtest)
ignore_external_warnings(gtest_main)
ignore_external_warnings(gmock)
ignore_external_warnings(gmock_main)

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

if(WIN32)
    add_custom_command(TARGET soundcoe_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                -t $<TARGET_FILE_DIR:soundcoe_tests>
                $<TARGET_RUNTIME_DLLS:soundcoe_tests>
        COMMAND_EXPAND_LISTS)
endif()
