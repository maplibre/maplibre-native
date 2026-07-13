function(find_static_library result_list_var)
    # Usage: find_static_library(LIST_VAR NAMES name1 [name2 ...])
    set(options)
    set(one_value_args)
    set(multi_value_args NAMES)
    cmake_parse_arguments(FSL "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT FSL_NAMES)
        message(FATAL_ERROR "find_static_library: NAMES argument required")
    endif()

    # Temporarily restrict to static libs
    set(_old_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

    foreach(lib_name IN LISTS FSL_NAMES)
        unset(_found_lib CACHE)
        find_library(_found_lib NAMES ${lib_name})
        if(_found_lib)
            message(STATUS "find_static_library: Found static [${lib_name}] -> ${_found_lib}")
            # Append to result list and return early
            set(${result_list_var} "${${result_list_var}};${_found_lib}" PARENT_SCOPE)
            set(CMAKE_FIND_LIBRARY_SUFFIXES ${_old_suffixes})
            return()
        endif()
    endforeach()

    # Restore original suffix list
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_old_suffixes})

    message(FATAL_ERROR "find_static_library: could not find any of the specified static libraries: ${FSL_NAMES}")
endfunction()
