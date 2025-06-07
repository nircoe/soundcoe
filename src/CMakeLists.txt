set(SOURCES
    core/audio_context.cpp
    core/error_handler.cpp
    resources/sound_buffer.cpp
    resources/sound_source.cpp
    resources/resource_manager.cpp
    playback/sound_effect.cpp
    playback/music.cpp
    playback/sound_player.cpp
    utils/audio_file.cpp
    utils/math.cpp
)

add_library(soundcoe ${SOURCES})

target_link_libraries(soundcoe 
    PUBLIC
        soundcoe_headers
    PRIVATE 
        OpenAL
        logcoe
)