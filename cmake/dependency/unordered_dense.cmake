# unordered_dense, as mbgl-unordered_dense: an installed one if it is at least as
# new as the vendored copy, unless MLN_USE_BUILTIN_UNORDERED_DENSE is set, and the
# vendored copy otherwise.

if(TARGET mbgl-unordered_dense)
    return()
endif()

if(NOT MLN_USE_BUILTIN_UNORDERED_DENSE)
    find_package(unordered_dense CONFIG QUIET)
endif()

if(unordered_dense_FOUND AND unordered_dense_VERSION VERSION_GREATER_EQUAL 4.1.2)
    message(STATUS "Using installed unordered_dense ${unordered_dense_VERSION}.")
    add_library(mbgl-unordered_dense ALIAS unordered_dense::unordered_dense)
else()
    set(MLN_USE_BUILTIN_UNORDERED_DENSE TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/unordered_dense.cmake)
    add_library(mbgl-unordered_dense ALIAS unordered_dense)
endif()
