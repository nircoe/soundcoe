add_library(soundcoe_headers INTERFACE)

target_include_directories(soundcoe_headers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>  
)

target_link_libraries(soundcoe_headers
    INTERFACE
        logcoe
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/soundcoe
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)