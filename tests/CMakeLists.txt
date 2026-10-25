soundcoe_ignore_external_warnings(gtest)
soundcoe_ignore_external_warnings(gtest_main)
soundcoe_ignore_external_warnings(gmock)
soundcoe_ignore_external_warnings(gmock_main)

add_executable(soundcoe_tests
    main.cpp
    core/audio_context_tests.cpp
    core/error_handler_tests.cpp
    core/error_tests.cpp
    core/types_tests.cpp
    resources/audio_data_tests.cpp
    resources/resource_manager_tests.cpp
    resources/sound_buffer_tests.cpp
    resources/sound_source_tests.cpp
    playback/sound_manager_tests.cpp
    utils/math_tests.cpp
)

target_link_libraries(soundcoe_tests
    PRIVATE
        soundcoe
        testcoe
        OpenAL
)

target_include_directories(soundcoe_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

soundcoe_enable_strict_warnings(soundcoe_tests)

if(WIN32)
    add_custom_command(TARGET soundcoe_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                -t $<TARGET_FILE_DIR:soundcoe_tests>
                $<TARGET_RUNTIME_DLLS:soundcoe_tests>
        COMMAND_EXPAND_LISTS)
endif()
