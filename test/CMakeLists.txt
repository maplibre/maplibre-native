add_library(
    mbgl-test STATIC EXCLUDE_FROM_ALL
    ${PROJECT_SOURCE_DIR}/test/actor/actor.test.cpp
    ${PROJECT_SOURCE_DIR}/test/actor/actor_ref.test.cpp
    ${PROJECT_SOURCE_DIR}/test/algorithm/update_renderables.test.cpp
    ${PROJECT_SOURCE_DIR}/test/algorithm/update_tile_masks.test.cpp
    ${PROJECT_SOURCE_DIR}/test/api/annotations.test.cpp
    ${PROJECT_SOURCE_DIR}/test/api/api_misuse.test.cpp
    ${PROJECT_SOURCE_DIR}/test/api/custom_geometry_source.test.cpp
    ${PROJECT_SOURCE_DIR}/test/api/query.test.cpp
    ${PROJECT_SOURCE_DIR}/test/api/recycle_map.cpp
    ${PROJECT_SOURCE_DIR}/test/geometry/dem_data.test.cpp
    ${PROJECT_SOURCE_DIR}/test/geometry/line_atlas.test.cpp
    ${PROJECT_SOURCE_DIR}/test/map/map.test.cpp
    ${PROJECT_SOURCE_DIR}/test/map/prefetch.test.cpp
    ${PROJECT_SOURCE_DIR}/test/map/transform.test.cpp
    ${PROJECT_SOURCE_DIR}/test/math/clamp.test.cpp
    ${PROJECT_SOURCE_DIR}/test/math/minmax.test.cpp
    ${PROJECT_SOURCE_DIR}/test/math/wrap.test.cpp
    ${PROJECT_SOURCE_DIR}/test/platform/settings.test.cpp
    ${PROJECT_SOURCE_DIR}/test/plugin/plugin.test.cpp
    ${PROJECT_SOURCE_DIR}/test/renderer/image_manager.test.cpp
    ${PROJECT_SOURCE_DIR}/test/renderer/pattern_atlas.test.cpp
    ${PROJECT_SOURCE_DIR}/test/renderer/shader_registry.test.cpp
    $<$<BOOL:${MLN_WITH_WEBGPU}>:${PROJECT_SOURCE_DIR}/test/renderer/wgsl_preprocessor.test.cpp>
    ${PROJECT_SOURCE_DIR}/test/sprite/sprite_loader.test.cpp
    ${PROJECT_SOURCE_DIR}/test/sprite/sprite_parser.test.cpp
    ${PROJECT_SOURCE_DIR}/test/src/mbgl/test/fixture_log_observer.cpp
    ${PROJECT_SOURCE_DIR}/test/src/mbgl/test/getrss.cpp
    ${PROJECT_SOURCE_DIR}/test/src/mbgl/test/sqlite3_test_fs.cpp
    ${PROJECT_SOURCE_DIR}/test/src/mbgl/test/stub_file_source.cpp
    ${PROJECT_SOURCE_DIR}/test/src/mbgl/test/test.cpp
    ${PROJECT_SOURCE_DIR}/test/src/mbgl/test/util.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/asset_file_source.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/database_file_source.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/headers.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/http_file_source.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/local_file_source.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/main_resource_loader.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/mbtiles_file_source.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/offline.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/offline_database.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/offline_download.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/online_file_source.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/pmtiles_file_source.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/resource.test.cpp
    ${PROJECT_SOURCE_DIR}/test/storage/sqlite.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/conversion_impl.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/function.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/geojson_options.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/layer.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/light.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/padding.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/property_value.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/source_options.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/stringify.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/conversion/tileset.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/expression/dependency.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/expression/expression.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/expression/util.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/filter.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/properties.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/property_expression.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/source.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/style.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/style_image.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/style_layer.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/style_parser.test.cpp
    ${PROJECT_SOURCE_DIR}/test/style/variable_anchor_offset_collection.test.cpp
    $<$<AND:$<NOT:$<BOOL:MBGL_WITH_QT>>,$<NOT:$<PLATFORM_ID:Windows>>>:${PROJECT_SOURCE_DIR}/test/text/bidi.test.cpp>
    ${PROJECT_SOURCE_DIR}/test/text/calculate_tile_distances.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/cross_tile_symbol_index.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/formatted.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/get_anchors.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/glyph_manager.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/glyph_pbf.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/language_tag.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/local_glyph_rasterizer.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/quads.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/shaping.test.cpp
    ${PROJECT_SOURCE_DIR}/test/text/tagged_string.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/custom_geometry_tile.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/geojson_tile.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/geometry_tile_data.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/raster_dem_tile.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/raster_tile.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/tile_cache.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/tile_coordinate.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/tile_id.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/tile_lod.test.cpp
    ${PROJECT_SOURCE_DIR}/test/tile/vector_tile.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/action_journal.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/async_task.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/bounding_volumes.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/camera.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/color.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/geo.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/grid_index.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/hash.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/http_timeout.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/image.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/mapbox.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/memory.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/merge_lines.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/number_conversions.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/padding.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/position.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/projection.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/rotation.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/run_loop.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/string.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/string_indexer.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/text_conversions.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/thread.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/thread_local.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/tile_cover.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/tile_range.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/timer.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/tiny_map.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/token.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/url.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/tile_server_options.test.cpp
    ${PROJECT_SOURCE_DIR}/test/util/lru_cache.test.cpp
)

# MapSnapshotter uses headless backend that is rendering image on a background thread. QT / macOS adaptation for headless backend creates
# new window and updates it's parameters, which is not allowed since macOS mohave 10.14. Following block disables snapshotter unit tests for
# QT on macOS. https://github.com/mapbox/mapbox-gl-native/issues/16267
if(NOT (MLN_WITH_QT AND CMAKE_SYSTEM_NAME STREQUAL Darwin))
    target_sources(
        mbgl-test
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/map/map_snapshotter.cpp ${PROJECT_SOURCE_DIR}/test/map/map_snapshotter.test.cpp
    )
endif()

if(MLN_WITH_OPENGL)
    target_sources(
        mbgl-test
        PRIVATE
            ${PROJECT_SOURCE_DIR}/test/api/custom_layer.test.cpp
            ${PROJECT_SOURCE_DIR}/test/api/custom_drawable_layer.test.cpp
            ${PROJECT_SOURCE_DIR}/test/gl/bucket.test.cpp
            ${PROJECT_SOURCE_DIR}/test/gl/enum.test.cpp
            ${PROJECT_SOURCE_DIR}/test/gl/context.test.cpp
            ${PROJECT_SOURCE_DIR}/test/gl/gl_functions.test.cpp
            ${PROJECT_SOURCE_DIR}/test/gl/object.test.cpp
            ${PROJECT_SOURCE_DIR}/test/gl/resource_pool.test.cpp
            ${PROJECT_SOURCE_DIR}/test/renderer/backend_scope.test.cpp
            ${PROJECT_SOURCE_DIR}/test/util/offscreen_texture.test.cpp
    )
endif()


if(MLN_WITH_VULKAN)
    target_sources(
        mbgl-test
        PRIVATE
            ${PROJECT_SOURCE_DIR}/test/api/custom_layer.test.cpp
            ${PROJECT_SOURCE_DIR}/test/api/custom_drawable_layer.test.cpp
            ${PROJECT_SOURCE_DIR}/test/renderer/backend_scope.test.cpp
            ${PROJECT_SOURCE_DIR}/test/util/offscreen_texture.test.cpp
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Android)
    message("Target platform does not support HTTP tests or dependencies not found.")

    set(MLN_TEST_HAS_TEST_SERVER 0)
else()
    set(MLN_TEST_HAS_TEST_SERVER 1)
    target_sources(
        mbgl-test
        PRIVATE ${PROJECT_SOURCE_DIR}/test/src/mbgl/test/http_server.cpp
    )
    set_source_files_properties(
        ${PROJECT_SOURCE_DIR}/test/src/mbgl/test/http_server.cpp
        PROPERTIES
        COMPILE_FLAGS
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wno-shadow>
    )
    set_source_files_properties(
        ${PROJECT_SOURCE_DIR}/test/src/mbgl/test/http_server.cpp
        PROPERTIES
        COMPILE_OPTIONS
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},iOS>:-Wno-shorten-64-to-32>
    )
endif()

if(NOT DEFINED ENV{CI})
    set(MLN_TEST_BUILD_ON_CI 0)
else()
    set(MLN_TEST_BUILD_ON_CI 1)
endif()

target_compile_definitions(
    mbgl-test
    PRIVATE TEST_HAS_SERVER=${MLN_TEST_HAS_TEST_SERVER} CI_BUILD=${MLN_TEST_BUILD_ON_CI}
)

target_include_directories(
    mbgl-test
    PRIVATE ${PROJECT_SOURCE_DIR}/platform/default/include ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/test/src
)

target_include_directories(
    mbgl-test
    PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/test/include
)

include(${PROJECT_SOURCE_DIR}/vendor/cpp-httplib.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/googletest.cmake)

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set_target_properties(mbgl-test mbgl-vendor-googletest PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH $<$<CONFIG:Debug>:YES>)
    set_target_properties(mbgl-test mbgl-vendor-googletest
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
endif()

# Needed for testing private classes
get_target_property(MLN_CORE_PRIVATE_LIBRARIES mbgl-core LINK_LIBRARIES)

target_link_libraries(
    mbgl-test
    PRIVATE
        ${MLN_CORE_PRIVATE_LIBRARIES}
        mbgl-vendor-args
        MapLibreNative::Base::pixelmatch-cpp
        mbgl-compiler-options
        mbgl-vendor-cpp-httplib
    PUBLIC mbgl-core
)

target_link_libraries(
    mbgl-test
    PUBLIC mbgl-vendor-googletest
    $<$<AND:$<PLATFORM_ID:Windows>,$<BOOL:MINGW>>:psapi>
)

if(WIN32)
    target_compile_definitions(
        mbgl-test
        PRIVATE MBGL_BUILDING_LIB
    )
endif()

set_property(TARGET mbgl-test PROPERTY FOLDER MapLibre)

if (MLN_WITH_CLANG_TIDY)
    set_target_properties(mbgl-test PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()

if (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL Linux OR MLN_WITH_WEBGPU)
    target_compile_definitions(
        mbgl-test
        PRIVATE USE_CPP_TEST_SERVER
    )
endif()
