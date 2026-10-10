function(soundcoe_ignore_external_warnings target_name)
    if(TARGET ${target_name})
        get_target_property(target_type ${target_name} TYPE)

        get_target_property(include_dirs ${target_name} INTERFACE_INCLUDE_DIRECTORIES)
        if(include_dirs)
            set_target_properties(${target_name} PROPERTIES
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${include_dirs}"
            )
        endif()

        if(target_type STREQUAL "STATIC_LIBRARY" OR
           target_type STREQUAL "SHARED_LIBRARY" OR
           target_type STREQUAL "EXECUTABLE")
            if(MSVC)
                target_compile_options(${target_name} PRIVATE /w)
            else()
                target_compile_options(${target_name} PRIVATE -w)
            endif()
        endif()

        message(STATUS "[soundcoe] Suppressing warnings for ${target_name} (${target_type})")
    else()
        message(WARNING "[soundcoe] Target ${target_name} not found")
    endif()
endfunction()

function(soundcoe_enable_strict_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target_name} PRIVATE -Werror -Wall -Wextra -Wpedantic)
    endif()
endfunction()
