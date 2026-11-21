# expected-lite, as mbgl-expected-lite: an installed one if it is at least as new
# as the vendored copy, unless MLN_USE_BUILTIN_EXPECTED_LITE is set, and the
# vendored copy otherwise.

if(TARGET mbgl-expected-lite)
    return()
endif()

if(NOT MLN_USE_BUILTIN_EXPECTED_LITE)
    find_package(expected-lite CONFIG QUIET)
endif()

if(expected-lite_FOUND AND expected-lite_VERSION VERSION_GREATER_EQUAL 0.10.0)
    message(STATUS "Using installed expected-lite ${expected-lite_VERSION}.")
    add_library(mbgl-expected-lite ALIAS nonstd::expected-lite)
else()
    set(MLN_USE_BUILTIN_EXPECTED_LITE TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/expected-lite.cmake)
    add_library(mbgl-expected-lite ALIAS mbgl-vendor-expected-lite)
endif()
