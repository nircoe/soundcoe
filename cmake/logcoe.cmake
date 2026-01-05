function(fetch_logcoe)
    if(NOT DEFINED SOUNDCOE_USE_LOGCOE)
        set(SOUNDCOE_USE_LOGCOE OFF)
    endif()

    if(SOUNDCOE_USE_LOGCOE)
        message(STATUS "[soundcoe] Using logcoe for logging")
        message(STATUS "[soundcoe] To disable: \"set(SOUNDCOE_USE_LOGCOE OFF)\" before fetching soundcoe")
        set(SOUNDCOE_USE_LOGCOE 1 PARENT_SCOPE)

        find_package(logcoe QUIET)

        if(NOT logcoe_FOUND)
            message(STATUS "[soundcoe] Fetching logcoe from source...")

            FetchContent_Declare(
                logcoe
                GIT_REPOSITORY https://github.com/nircoe/logcoe.git
                GIT_TAG v0.1.1
            )
            FetchContent_MakeAvailable(logcoe)
            ignore_external_warnings(logcoe)
        else()
            message(STATUS "[soundcoe] Using existing logcoe")
        endif()
    else()
        message(STATUS "[soundcoe] logcoe disabled")
        message(STATUS "[soundcoe] To enable: \"set(SOUNDCOE_USE_LOGCOE ON)\" before fetching soundcoe")
        set(SOUNDCOE_USE_LOGCOE 0 PARENT_SCOPE)
    endif()
endfunction()