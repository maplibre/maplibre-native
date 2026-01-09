if(TARGET mbgl-freetype)
    return()
endif()
if (MLN_TEXT_SHAPING_HARFBUZZ)
    add_library(mbgl-freetype STATIC
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/autofit/autofit.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftbase.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftbbox.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftbdf.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftbitmap.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftcid.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftfstype.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftgasp.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftglyph.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftgxval.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftinit.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftmm.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftotval.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftpatent.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftpfr.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftstroke.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftsynth.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/fttype1.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftwinfnt.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/bdf/bdf.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/bzip2/ftbzip2.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/cache/ftcache.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/cff/cff.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/cid/type1cid.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/gzip/ftgzip.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/lzw/ftlzw.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/pcf/pcf.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/pfr/pfr.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/psaux/psaux.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/pshinter/pshinter.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/psnames/psnames.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/raster/raster.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/sdf/sdf.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/sfnt/sfnt.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/smooth/smooth.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/svg/svg.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/truetype/truetype.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/type1/type1.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/type42/type42.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/winfonts/winfnt.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftsystem.c
        ${CMAKE_CURRENT_LIST_DIR}/freetype/src/base/ftdebug.c
    )

    target_include_directories(mbgl-freetype
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/freetype/include
    )

    target_compile_definitions(mbgl-freetype PRIVATE
        FT2_BUILD_LIBRARY
        FT_DISABLE_BROTLI
        FT_REQUIRE_BROTLI=0
        FT_DISABLE_ZLIB
        FT_REQUIRE_ZLIB=0
    )

    set_target_properties(
        mbgl-freetype
        PROPERTIES
            INTERFACE_MAPLIBRE_NAME "freetype"
            INTERFACE_MAPLIBRE_URL "https://github.com/freetype/freetype"
            INTERFACE_MAPLIBRE_AUTHOR "David Turner, Robert Wilhelm, Werner Lemberg and FreeType contributors"
            INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/freetype/docs/FTL.TXT
    )
endif()
