# Boost, as mbgl-boost: installed headers if they are at least as new as the
# vendored copy, unless MLN_USE_BUILTIN_BOOST is set, and the vendored copy
# otherwise.

if(TARGET mbgl-boost)
    return()
endif()

if(NOT MLN_USE_BUILTIN_BOOST)
    find_package(Boost CONFIG QUIET)
endif()

if(Boost_FOUND AND Boost_VERSION VERSION_GREATER_EQUAL 1.90)
    message(STATUS "Using installed Boost ${Boost_VERSION}.")
    add_library(mbgl-boost ALIAS Boost::headers)
else()
    set(MLN_USE_BUILTIN_BOOST TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/boost.cmake)
    add_library(mbgl-boost ALIAS mbgl-vendor-boost)
endif()
