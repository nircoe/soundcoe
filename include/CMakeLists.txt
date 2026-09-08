add_library(soundcoe_headers INTERFACE)

target_include_directories(soundcoe_headers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${SOUNDCOE_CONFIG_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(soundcoe_headers
    INTERFACE
        $<$<BOOL:${SOUNDCOE_USE_LOGCOE}>:logcoe>
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/soundcoe.hpp
    DESTINATION include
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/soundcoe
    DESTINATION include/soundcoe
    FILES_MATCHING PATTERN "*.hpp"
)
