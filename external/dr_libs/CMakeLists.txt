add_library(dr_libs STATIC dr_wav_impl.cpp dr_mp3_impl.cpp)

target_include_directories(dr_libs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
