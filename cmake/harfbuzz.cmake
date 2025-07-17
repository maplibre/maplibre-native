set(SHAPE_TARGETS, "")
if (MLN_TEXT_SHAPING_HARFBUZZ)
    target_compile_definitions(
        mbgl-core
        PRIVATE MLN_TEXT_SHAPING_HARFBUZZ=1
    )
    list(APPEND
        SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/mbgl/text/harfbuzz.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/text/harfbuzz.hpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/text/harfbuzz_impl.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/text/harfbuzz_impl.hpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/text/freetype.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/text/freetype.hpp
    )

    set(SHAPE_TARGETS
        freetype
        harfbuzz
    )

    set(FT_DISABLE_BROTLI ON CACHE BOOL "freetype option")
    set(FT_REQUIRE_BROTLI OFF CACHE BOOL "freetype option")
    set(FT_DISABLE_ZLIB ON CACHE BOOL "freetype option")
    set(FT_REQUIRE_ZLIB OFF CACHE BOOL "freetype option")
    add_subdirectory(vendor/freetype)
    add_subdirectory(vendor/harfbuzz)

    target_compile_definitions(harfbuzz PRIVATE -DHAVE_FREETYPE)

    set_target_properties(
        freetype
        PROPERTIES
            INTERFACE_MAPLIBRE_NAME "freetype"
            INTERFACE_MAPLIBRE_URL "https://github.com/freetype/freetype"
            INTERFACE_MAPLIBRE_AUTHOR "David Turner, Robert Wilhelm, Werner Lemberg and FreeType contributors"
            INTERFACE_MAPLIBRE_LICENSE ${PROJECT_SOURCE_DIR}/vendor/freetype/docs/FTL.TXT
    )

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
        PUBLIC vendor/freetype/include
    )

    target_include_directories(
        mbgl-core SYSTEM
        PUBLIC vendor/harfbuzz/src
    )
endif()
