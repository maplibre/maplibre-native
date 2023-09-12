cmake_minimum_required(VERSION 2.8.12)
project(harfbuzzFreetype)

option(BAZEL_BUILD "Bazel build option" OFF)

if(BAZEL_BUILD) 

    set (project_headers harfbuzz/src/hb-ft.h)
endif()
set(FT_DISABLE_BROTLI ON CACHE BOOL "freetype option")
set(FT_REQUIRE_BROTLI OFF CACHE BOOL "freetype option")
add_subdirectory(freetype)
add_subdirectory(harfbuzz)

target_compile_definitions(harfbuzz PRIVATE -DHAVE_FREETYPE)

set_target_properties(
    freetype
    PROPERTIES
        INTERFACE_MAPBOX_NAME "freetype"
        INTERFACE_MAPBOX_URL "https://github.com/freetype/freetype"
        INTERFACE_MAPBOX_AUTHOR "David Turner"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/freetype/docs/FTL.TXT
)

set_target_properties(
    harfbuzz
    PROPERTIES
        INTERFACE_MAPBOX_NAME "harfbuzz"
        INTERFACE_MAPBOX_URL "https://github.com/harfbuzz/harfbuzz"
        INTERFACE_MAPBOX_AUTHOR  ${CMAKE_CURRENT_LIST_DIR}/harfbuzz/AUTHORS
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/harfbuzz/COPYING
)

add_library(harfbuzzFreetype STATIC freetype_wrap.cpp harfbuzz_wrap.cpp)

set_target_properties(
    harfbuzzFreetype
    PROPERTIES
        INTERFACE_MAPBOX_NAME "Harfbuzz Freetype wrap"
        INTERFACE_MAPBOX_URL "https://github.com/maplibre/maplibre-native"
        INTERFACE_MAPBOX_AUTHOR "Alan Wei Chen"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/harfbuzz/COPYING
)

target_include_directories(harfbuzzFreetype PRIVATE  harfbuzz/src/
                                                     freetype/include/)

target_link_libraries(harfbuzzFreetype PRIVATE freetype harfbuzz)

set_property(TARGET harfbuzz PROPERTY FOLDER Core)
set_property(TARGET freetype PROPERTY FOLDER Core)
set_property(TARGET harfbuzzFreetype PROPERTY FOLDER Core)

target_include_directories(harfbuzzFreetype SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)


install(TARGETS harfbuzzFreetype ARCHIVE DESTINATION lib)

install(FILES freetype_wrap.hpp harfbuzz_wrap.hpp DESTINATION include)

target_compile_options(
    harfbuzz
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)