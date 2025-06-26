set(SOURCES
    core/audio_context.cpp
    core/error_handler.cpp
    resources/audio_data.cpp
    resources/sound_buffer.cpp
    resources/sound_source.cpp
    resources/resource_manager.cpp
    playback/sound_effect.cpp
    playback/music.cpp
    playback/sound_player.cpp
    utils/math.cpp
)

add_library(soundcoe ${SOURCES})

target_include_directories(soundcoe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../external
)

target_link_libraries(soundcoe 
    PUBLIC
        soundcoe_headers
    PRIVATE 
        OpenAL
        logcoe
)