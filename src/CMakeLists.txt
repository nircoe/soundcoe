set(SOURCES
    core/audio_context.cpp
    core/error_handler.cpp
    resources/audio_data.cpp
    resources/sound_buffer.cpp
    resources/sound_source.cpp
    resources/resource_manager.cpp
    playback/sound_manager.cpp
    utils/math.cpp
    soundcoe.cpp
)

add_library(soundcoe ${SOURCES})

target_include_directories(soundcoe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../external
)

# Link libraries based on platform
if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_link_libraries(soundcoe 
        PUBLIC
            soundcoe_headers
            Threads::Threads
        PRIVATE
            $<$<BOOL:${SOUNDCOE_USE_LOGCOE}>:logcoe>
    )
    # Link with Emscripten's built-in OpenAL implementation
    target_link_options(soundcoe PRIVATE -lopenal)
else()
    target_link_libraries(soundcoe
        PUBLIC
            soundcoe_headers
            Threads::Threads
        PRIVATE
            OpenAL
            $<$<BOOL:${SOUNDCOE_USE_LOGCOE}>:logcoe>
    )
endif()

if(MINGW)
    target_link_options(soundcoe INTERFACE -static-libgcc -static-libstdc++ -static)
endif()
