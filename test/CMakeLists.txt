add_library(
    mbgl-test STATIC EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_LIST_DIR}/actor/actor.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/actor/actor_ref.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/algorithm/update_renderables.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/algorithm/update_tile_masks.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/annotations.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/api_misuse.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/custom_geometry_source.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/location_indicator.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/query.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/recycle_map.cpp
    ${CMAKE_CURRENT_LIST_DIR}/geometry/dem_data.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/geometry/line_atlas.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/map/map.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/map/prefetch.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/map/transform.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/math/clamp.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/math/minmax.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/math/wrap.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/platform/settings.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/plugin/plugin.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/dynamic_texture_atlas.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/fill_bucket.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/feature_tracking.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/hillshade_layer.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/image_manager.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/pattern_atlas.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/renderable.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/shader_registry.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer/source_state.test.cpp
    $<$<BOOL:${MLN_WITH_WEBGPU}>:${CMAKE_CURRENT_LIST_DIR}/renderer/wgsl_preprocessor.test.cpp>
    ${CMAKE_CURRENT_LIST_DIR}/sprite/sprite_loader.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sprite/sprite_parser.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/test/fixture_log_observer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/test/getrss.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/test/sqlite3_test_fs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/test/stub_file_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/test/test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mln/test/util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/asset_file_source.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/database_file_source.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/headers.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/http_file_source.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/local_file_source.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/main_resource_loader.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/mbtiles_file_source.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/offline.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/offline_database.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/offline_download.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/online_file_source.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/pmtiles_file_source.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/resource.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/storage/sqlite.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/conversion_impl.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/function.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/geojson_options.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/layer.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/light.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/padding.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/property_value.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/source_options.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/stringify.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/conversion/tileset.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/expression/dependency.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/expression/expression.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/expression/semiliteral.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/expression/global_state.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/expression/util.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/expression/utf8_op_helpers.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/filter.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/properties.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/property_expression.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/source.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/style.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/style_image.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/style_layer.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/style_parser.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style/variable_anchor_offset_collection.test.cpp
    $<$<AND:$<NOT:$<BOOL:MBGL_WITH_QT>>,$<NOT:$<PLATFORM_ID:Windows>>>:${CMAKE_CURRENT_LIST_DIR}/text/bidi.test.cpp>
    ${CMAKE_CURRENT_LIST_DIR}/text/calculate_tile_distances.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/cross_tile_symbol_index.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/formatted.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/get_anchors.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/glyph_manager.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/glyph_pbf.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/language_tag.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/local_glyph_rasterizer.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/quads.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/shaping.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/text/tagged_string.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/custom_geometry_tile.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/geojson_tile.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/geometry_tile_data.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/raster_dem_tile.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/raster_tile.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/tile_cache.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/tile_coordinate.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/tile_id.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/tile_lod.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile/vector_tile.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/action_journal.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/async_task.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/bounding_volumes.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/camera.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/color.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/geo.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/grid_index.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/hash.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/http_timeout.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/image.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/mapbox.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/mat3.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/memory.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/merge_lines.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/number_conversions.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/padding.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/position.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/projection.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/rotation.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/run_loop.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/string.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/string_indexer.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/text_conversions.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/thread.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/thread_local.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/tile_cover.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/tile_range.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/timer.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/tiny_map.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/token.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/url.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/tile_server_options.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/lru_cache.test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/vectors.test.cpp
)

# MapSnapshotter uses headless backend that is rendering image on a background thread. QT / macOS adaptation for headless backend creates
# new window and updates it's parameters, which is not allowed since macOS mohave 10.14. Following block disables snapshotter unit tests for
# QT on macOS. https://github.com/mapbox/mapbox-gl-native/issues/16267
if(NOT (MLN_WITH_QT AND CMAKE_SYSTEM_NAME STREQUAL Darwin))
    target_sources(
        mbgl-test
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mln/map/map_snapshotter.cpp ${CMAKE_CURRENT_LIST_DIR}/map/map_snapshotter.test.cpp
    )
endif()

if(MLN_WITH_OPENGL)
    target_sources(
        mbgl-test
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/api/custom_layer.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/gl/bucket.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/gl/enum.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/gl/context.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/gl/drawable.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/gl/gl_functions.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/gl/object.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/gl/resource_pool.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/renderer/backend_scope.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/util/offscreen_texture.test.cpp
    )
endif()


if(MLN_WITH_METAL)
    target_sources(
        mbgl-test
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/renderer/metal_renderable.test.cpp
    )
endif()

if(MLN_WITH_VULKAN)
    target_sources(
        mbgl-test
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/api/custom_layer.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/renderer/backend_scope.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/renderer/vulkan_acquire.test.cpp
            ${CMAKE_CURRENT_LIST_DIR}/util/offscreen_texture.test.cpp
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Android)
    message("Target platform does not support HTTP tests or dependencies not found.")

    set(MLN_TEST_HAS_TEST_SERVER 0)
else()
    set(MLN_TEST_HAS_TEST_SERVER 1)
    target_sources(
        mbgl-test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/mln/test/http_server.cpp
    )
    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/src/mln/test/http_server.cpp
        PROPERTIES
        COMPILE_FLAGS
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wno-shadow>
    )
    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/src/mln/test/http_server.cpp
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
    PRIVATE ${PROJECT_SOURCE_DIR}/platform/default/include ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/src
)

target_include_directories(
    mbgl-test
    PUBLIC ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/include
)

include(${PROJECT_SOURCE_DIR}/vendor/cpp-httplib.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/googletest.cmake)

if(MLN_WITH_PLUGINS)
    target_sources(mbgl-test PRIVATE
        plugin/api.test.cpp
        plugin/layout.test.cpp
        plugin/property_binding.test.cpp
        plugin/registry.test.cpp
        plugin/rendering.test.cpp
    )
endif()

mln_test_library_properties(mbgl-test mbgl-vendor-googletest)

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
