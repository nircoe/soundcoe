function(ignore_external_warnings target_name)
    if(TARGET ${target_name})
        get_target_property(INCLUDE_DIRS ${target_name} INTERFACE_INCLUDE_DIRECTORIES)
        if(INCLUDE_DIRS)
            set_target_properties(${target_name} PROPERTIES
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${INCLUDE_DIRS}"
            )
            message(STATUS "Ignoring ${target_name} warnings")
        endif()
    else()
        message(WARNING "Target ${target_name} not found")
    endif()
endfunction()