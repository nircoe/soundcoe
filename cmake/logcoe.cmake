function(soundcoe_fetch_logcoe)
    if(NOT DEFINED SOUNDCOE_USE_LOGCOE)
        set(SOUNDCOE_USE_LOGCOE OFF)
    endif()

    if(NOT SOUNDCOE_USE_LOGCOE)
        message(STATUS "[soundcoe] logcoe disabled")
        message(STATUS "[soundcoe] To enable: \"set(SOUNDCOE_USE_LOGCOE ON)\" before fetching soundcoe")
        set(SOUNDCOE_USE_LOGCOE 0 PARENT_SCOPE)
        return()
    endif()

    message(STATUS "[soundcoe] Using logcoe for logging")
    message(STATUS "[soundcoe] To disable: \"set(SOUNDCOE_USE_LOGCOE OFF)\" before fetching soundcoe")
    set(SOUNDCOE_USE_LOGCOE 1 PARENT_SCOPE)

    if(TARGET logcoe)
        message(STATUS "[soundcoe] logcoe already available, skipping fetch")
        return()
    endif()

    message(STATUS "[soundcoe] Fetching logcoe from source...")

    FetchContent_Declare(
        logcoe
        GIT_REPOSITORY https://github.com/nircoe/logcoe.git
        GIT_TAG v0.1.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(logcoe)
    soundcoe_ignore_external_warnings(logcoe)
endfunction()
