soundcoe_ignore_external_warnings(gtest)
soundcoe_ignore_external_warnings(gtest_main)
soundcoe_ignore_external_warnings(gmock)
soundcoe_ignore_external_warnings(gmock_main)

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

soundcoe_enable_strict_warnings(soundcoe_tests)

if(WIN32)
    add_custom_command(TARGET soundcoe_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                -t $<TARGET_FILE_DIR:soundcoe_tests>
                $<TARGET_RUNTIME_DLLS:soundcoe_tests>
        COMMAND_EXPAND_LISTS)
endif()
