# HarfBuzz, and the FreeType it is built with: installed ones if there are both,
# at least as new as the vendored copies, unless MLN_USE_BUILTIN_FREETYPE or
# MLN_USE_BUILTIN_HARFBUZZ is set, and the vendored copies otherwise. They go
# together, as the vendored HarfBuzz is built with the vendored FreeType, and
# mixing them would link two FreeTypes.

if(TARGET mbgl-harfbuzz)
    return()
endif()

if(NOT MLN_USE_BUILTIN_FREETYPE AND NOT MLN_USE_BUILTIN_HARFBUZZ)
    find_package(Freetype QUIET)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_search_module(HARFBUZZ IMPORTED_TARGET harfbuzz)
    endif()
endif()

if(FREETYPE_FOUND
   AND FREETYPE_VERSION_STRING VERSION_GREATER_EQUAL 2.13.3
   AND HARFBUZZ_FOUND
   AND HARFBUZZ_VERSION VERSION_GREATER_EQUAL 8.2.0)
    message(STATUS "Using installed FreeType ${FREETYPE_VERSION_STRING} and HarfBuzz ${HARFBUZZ_VERSION}.")
    add_library(mbgl-freetype INTERFACE IMPORTED)
    target_link_libraries(mbgl-freetype INTERFACE Freetype::Freetype)
    add_library(mbgl-harfbuzz INTERFACE IMPORTED)
    target_link_libraries(mbgl-harfbuzz INTERFACE PkgConfig::HARFBUZZ)
else()
    set(MLN_USE_BUILTIN_FREETYPE TRUE)
    set(MLN_USE_BUILTIN_HARFBUZZ TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/freetype.cmake)
    include(${PROJECT_SOURCE_DIR}/vendor/harfbuzz.cmake)
endif()
