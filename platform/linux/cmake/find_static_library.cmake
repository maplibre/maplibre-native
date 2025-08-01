function(find_static_library result_var)
    # Usage: find_static_library(VAR_NAME NAMES name1 [name2 ...])
    set(options)
    set(one_value_args)
    set(multi_value_args NAMES)
    cmake_parse_arguments(FSL "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT FSL_NAMES)
        message(FATAL_ERROR "find_static_library: NAMES argument required")
    endif()

    # Save current suffixes
    set(_old_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    find_library(_found_static_lib NAMES ${FSL_NAMES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_old_suffixes})

    if(_found_static_lib)
        set(${result_var} "${_found_static_lib}" PARENT_SCOPE)
        message(STATUS "Found static library for ${FSL_NAMES}: ${_found_static_lib}")
    else()
        message(FATAL_ERROR "Static library (${FSL_NAMES}) not found")
    endif()
endfunction()
