function(find_static_library result_var)
    # Usage: find_static_library(VAR_NAME NAMES name1 [name2 ...])
    set(options)
    set(one_value_args)
    set(multi_value_args NAMES)
    cmake_parse_arguments(FSL "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT FSL_NAMES)
        message(FATAL_ERROR "find_static_library: NAMES argument required")
    endif()

    # Clear any previous cached result for this variable
    unset(${result_var} CACHE)

    # Temporarily force CMake to look only for .a
    set(_old_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

    # Invoke find_library using the caller's var name
    find_library(${result_var} NAMES ${FSL_NAMES})

    # Restore original suffix list
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_old_suffixes})

    # Check result
    if(NOT ${result_var})
        message(FATAL_ERROR
            "find_static_library: could not find any of [${FSL_NAMES}] as a .a")
    endif()

    message(STATUS
        "find_static_library: Found static [${FSL_NAMES}] -> ${${result_var}}")
endfunction()
