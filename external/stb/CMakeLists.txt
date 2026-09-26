add_library(stb_vorbis STATIC stb_vorbis.c)

target_include_directories(stb_vorbis
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
