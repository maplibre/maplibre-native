if(TARGET freetype)
    return()
endif()
if (MLN_TEXT_SHAPING_HARFBUZZ)
    set(FT_DISABLE_BROTLI ON CACHE BOOL "freetype option")
    set(FT_REQUIRE_BROTLI OFF CACHE BOOL "freetype option")
    set(FT_DISABLE_ZLIB ON CACHE BOOL "freetype option")
    set(FT_REQUIRE_ZLIB OFF CACHE BOOL "freetype option")
    add_subdirectory(vendor/freetype SYSTEM)

    set_target_properties(
        freetype
        PROPERTIES
            INTERFACE_MAPLIBRE_NAME "freetype"
            INTERFACE_MAPLIBRE_URL "https://github.com/freetype/freetype"
            INTERFACE_MAPLIBRE_AUTHOR "David Turner, Robert Wilhelm, Werner Lemberg and FreeType contributors"
            INTERFACE_MAPLIBRE_LICENSE ${PROJECT_SOURCE_DIR}/vendor/freetype/docs/FTL.TXT
    )

    target_include_directories(
        mbgl-core SYSTEM
        PUBLIC vendor/freetype/include
    )
endif()
