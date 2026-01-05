function(generate_soundcoe_config_header)
    set(SOUNDCOE_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/config")
    set(SOUNDCOE_CONFIG_DIR ${SOUNDCOE_CONFIG_DIR} PARENT_SCOPE)

    file(MAKE_DIRECTORY ${SOUNDCOE_CONFIG_DIR})

    configure_file(
        ${SOUNDCOE_SOURCE_ROOT_DIR}/cmake/soundcoe_config.hpp.in
        ${SOUNDCOE_CONFIG_DIR}/soundcoe_config.hpp
        @ONLY
    )
endfunction()