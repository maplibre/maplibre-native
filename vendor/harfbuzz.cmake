if(TARGET harfbuzz)
    return()
endif()
if (MLN_TEXT_SHAPING_HARFBUZZ)
    add_subdirectory(vendor/harfbuzz SYSTEM)

    target_compile_definitions(harfbuzz PRIVATE -DHAVE_FREETYPE)

    set_target_properties(
        harfbuzz
        PROPERTIES
            INTERFACE_MAPLIBRE_NAME "harfbuzz"
            INTERFACE_MAPLIBRE_URL "https://github.com/harfbuzz/harfbuzz"
            INTERFACE_MAPLIBRE_AUTHOR ${PROJECT_SOURCE_DIR}/vendor/harfbuzz/AUTHORS
            INTERFACE_MAPLIBRE_LICENSE ${PROJECT_SOURCE_DIR}/vendor/harfbuzz/COPYING
    )

    target_include_directories(
        mbgl-core SYSTEM
        PUBLIC vendor/harfbuzz/src
    )
endif()
