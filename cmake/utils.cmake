function(ignore_external_warnings target_name)
    if(TARGET ${target_name})
        # Check if this is a compilable target type
        get_target_property(TARGET_TYPE ${target_name} TYPE)
        
        # Make headers system headers to suppress include warnings
        get_target_property(INCLUDE_DIRS ${target_name} INTERFACE_INCLUDE_DIRECTORIES)
        if(INCLUDE_DIRS)
            set_target_properties(${target_name} PROPERTIES
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${INCLUDE_DIRS}"
            )
        endif()
        
        # Only apply compile options to compilable targets, use conservative approach
        if(TARGET_TYPE STREQUAL "STATIC_LIBRARY" OR TARGET_TYPE STREQUAL "SHARED_LIBRARY" OR TARGET_TYPE STREQUAL "EXECUTABLE")
            # Just add warning suppression without removing existing flags to avoid breaking generator expressions
            if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                target_compile_options(${target_name} PRIVATE -w)  # Suppress all warnings
            elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                target_compile_options(${target_name} PRIVATE /w)  # Suppress all warnings
            endif()
        endif()
        
        message(STATUS "Suppressing warnings for ${target_name} (${TARGET_TYPE})")
    else()
        message(WARNING "Target ${target_name} not found")
    endif()
endfunction()

# Function to recursively ignore warnings for all targets in a directory
function(ignore_external_warnings_recursive dir_name)
    # Get all targets in this directory
    get_property(targets DIRECTORY ${dir_name} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target ${targets})
        ignore_external_warnings(${target})
    endforeach()
    
    # Get all subdirectories and recurse
    get_property(subdirs DIRECTORY ${dir_name} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirs})
        ignore_external_warnings_recursive(${subdir})
    endforeach()
endfunction()