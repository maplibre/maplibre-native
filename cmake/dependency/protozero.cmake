# protozero, as mbgl-protozero: installed headers if they are at least as new as
# the vendored copy, unless MLN_USE_BUILTIN_PROTOZERO is set, and the vendored copy
# otherwise. protozero installs no CMake or pkg-config files, so its headers are
# found directly, and its version read from them.

if(TARGET mbgl-protozero)
    return()
endif()

if(NOT MLN_USE_BUILTIN_PROTOZERO)
    find_path(PROTOZERO_INCLUDE_DIR protozero/version.hpp)
endif()

if(PROTOZERO_INCLUDE_DIR)
    file(STRINGS ${PROTOZERO_INCLUDE_DIR}/protozero/version.hpp protozero_version
         REGEX "#define PROTOZERO_VERSION_STRING")
    string(REGEX MATCH "[0-9.]+" protozero_version "${protozero_version}")
endif()

if(PROTOZERO_INCLUDE_DIR AND protozero_version VERSION_GREATER_EQUAL 1.7.1)
    message(STATUS "Using installed protozero ${protozero_version}.")
    add_library(mbgl-protozero INTERFACE IMPORTED)
    target_include_directories(mbgl-protozero SYSTEM INTERFACE ${PROTOZERO_INCLUDE_DIR})
else()
    set(MLN_USE_BUILTIN_PROTOZERO TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/protozero.cmake)
    add_library(mbgl-protozero ALIAS mbgl-vendor-protozero)
endif()
