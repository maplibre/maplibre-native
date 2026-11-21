# nunicode, as mbgl-nunicode: an installed one, unless MLN_USE_BUILTIN_NUNICODE is
# set, and the vendored copy otherwise. Any installed one will do, as nunicode
# installs no version file, and the vendored copy is an old one, 1.8.

if(TARGET mbgl-nunicode)
    return()
endif()

if(NOT MLN_USE_BUILTIN_NUNICODE)
    find_package(nunicode CONFIG QUIET)
endif()

if(nunicode_FOUND)
    message(STATUS "Using installed nunicode.")
    add_library(mbgl-nunicode ALIAS nunicode::nu)
    # nunicode installs no pkg-config module.
    mln_pkg_config_libs(-lnu)
else()
    set(MLN_USE_BUILTIN_NUNICODE TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/nunicode.cmake)
    add_library(mbgl-nunicode ALIAS mbgl-vendor-nunicode)
endif()
