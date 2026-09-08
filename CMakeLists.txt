cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

option(MLN_WITH_CORE_ONLY "Build only the core bits, no platform code" OFF)
option(MLN_WITH_CLANG_TIDY "Build with clang-tidy checks enabled" OFF)
option(MLN_WITH_COVERAGE "Enable code coverage collection" OFF)
option(MLN_WITH_QT "Build MapLibre Native Qt bindings" OFF)
option(MLN_WITH_NODE "Build MapLibre Native Node.js bindings" OFF)
option(MLN_WITH_GLFW "Set up targets for GLFW platform" ON)
option(MLN_WITH_SANITIZER "Use [address|thread|undefined] here" OFF)
option(MLN_WITH_RTTI "Compile with runtime type information" OFF)
option(MLN_WITH_OPENGL "Build with OpenGL renderer" OFF)
option(MLN_WITH_EGL "Build with EGL renderer" OFF)
option(MLN_WITH_VULKAN "Build with Vulkan renderer" OFF)
option(MLN_WITH_METAL "Build with Metal renderer" OFF)
option(MLN_WITH_WEBGPU "Build with WebGPU renderer" OFF)
option(MLN_WITH_PMTILES "Build with PMTiles support" ON)
option(MLN_WITH_WERROR "Make all compilation warnings errors" ON)
option(MLN_USE_UNORDERED_DENSE "Use ankerl dense containers for performance" ON)
option(MLN_USE_TRACY "Enable Tracy instrumentation" OFF)
option(MLN_USE_RUST "Use components in Rust" OFF)
option(MLN_TEXT_SHAPING_HARFBUZZ "Use haffbuzz to shape complex text" ON)
option(MLN_CREATE_AUTORELEASEPOOL "Create autoreleasepool in render loop" OFF)
option(MLN_CREATE_AMALGAMATION "Create static amalgamation of core (requires amerge)" OFF)

include(cmake/validate-backend-options.cmake)
include(cmake/clang-tidy.cmake)

if (MLN_WITH_QT AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)
endif()

if(WIN32 AND NOT DEFINED ENV{MSYSTEM} AND NOT MLN_WITH_QT AND NOT CMAKE_SYSTEM_NAME STREQUAL Android)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/platform/windows/custom-toolchain.cmake)
endif()

project("MapLibre Native" LANGUAGES CXX C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER MapLibre)

include(CTest)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(
    mbgl-compiler-options INTERFACE
)

set(UBSAN_BLACKLIST ${PROJECT_SOURCE_DIR}/scripts/ubsan.blacklist)

if (MLN_WITH_SANITIZER)
    message(STATUS "MLN_WITH_SANITIZER is enabled, using \"${MLN_WITH_SANITIZER}\" sanitizer")
endif()

target_compile_options(
    mbgl-compiler-options
    INTERFACE
        $<$<BOOL:${MLN_WITH_COVERAGE}>:--coverage>
        $<$<BOOL:${MLN_WITH_SANITIZER}>:-DSANITIZE>
        $<$<BOOL:${MLN_WITH_SANITIZER}>:-fno-omit-frame-pointer>
        $<$<BOOL:${MLN_WITH_SANITIZER}>:-fno-optimize-sibling-calls>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},address>:-fsanitize=address>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},thread>:-fsanitize=thread>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},undefined>:-fsanitize=implicit-conversion,
        -fsanitize-blacklist=${UBSAN_BLACKLIST}>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},undefined>:-fsanitize=undefined,
        -fsanitize-blacklist=${UBSAN_BLACKLIST}>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},undefined>:-fsanitize=unsigned-integer-overflow,
        -fsanitize-blacklist=${UBSAN_BLACKLIST}>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},undefined>:-fsanitize=nullability,
        -fsanitize-blacklist=${UBSAN_BLACKLIST}>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},undefined>:-fsanitize=float-divide-by-zero,
        -fsanitize-blacklist=${UBSAN_BLACKLIST}>
        $<$<PLATFORM_ID:iOS>:-fembed-bitcode>
        $<$<AND:$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>,$<NOT:$<OR:$<BOOL:${MLN_WITH_RTTI}>,$<CXX_COMPILER_ID:MSVC>>>>:-fno-rtti>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<PLATFORM_ID:Windows>>>:-Wall>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<PLATFORM_ID:Windows>>>:-Wshadow>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<PLATFORM_ID:Windows>>>:-Wextra>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<PLATFORM_ID:Windows>>,$<BOOL:${MLN_WITH_WERROR}>>:-Werror>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<PLATFORM_ID:Android>>:-Wno-error=tautological-constant-compare>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-error=maybe-uninitialized>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-error=return-type>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-error=unknown-pragmas>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-error=pragmas>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-psabi>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wno-unknown-warning-option>
        $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<NOT:$<BOOL:${MLN_WITH_QT}>>>:-Wno-error=deprecated-declarations>
        $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<NOT:$<BOOL:${MLN_WITH_QT}>>>:-Wno-error=unused-parameter>
        $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<NOT:$<BOOL:${MLN_WITH_QT}>>>:-Wno-error=unused-property-ivar>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        $<$<CXX_COMPILER_ID:MSVC>:/WX>      # all warnings as errors
        $<$<CXX_COMPILER_ID:MSVC>:/EHsc>    # exceptions
        $<$<CXX_COMPILER_ID:MSVC>:/wd4068>  # pragma
        $<$<CXX_COMPILER_ID:MSVC>:/D_USE_MATH_DEFINES>
        $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<PLATFORM_ID:Windows>>:-D_USE_MATH_DEFINES>
        $<$<CXX_COMPILER_ID:MSVC>:/D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
        $<$<CXX_COMPILER_ID:MSVC>:/D_CRT_SECURE_NO_WARNINGS>
)

target_link_libraries(
    mbgl-compiler-options
    INTERFACE
        $<$<BOOL:${MLN_WITH_COVERAGE}>:--coverage>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},address>:-fsanitize=address>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},thread>:-fsanitize=thread>
        $<$<STREQUAL:${MLN_WITH_SANITIZER},undefined>:-fsanitize=undefined>
)

cmake_policy(SET CMP0063 NEW)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
if (CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    set(CMAKE_CROSS_CONFIGS "all")
    set(CMAKE_DEFAULT_CONFIGS "all")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if(MLN_WITH_QT)
    add_library(mbgl-core OBJECT)
else()
    add_library(mbgl-core STATIC)
endif()

# Override visibility for mbgl-core when building amalgamation
if(MLN_CREATE_AMALGAMATION)
    target_compile_options(mbgl-core PRIVATE
        -fvisibility=default
    )
endif()

list(APPEND INCLUDE_FILES
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/context.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/drawable.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/drawable_data.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/drawable_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/drawable_builder.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/drawable_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/drawable_atlases_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/drawable_custom_layer_host_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/dynamic_texture.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/dynamic_texture_atlas.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/gpu_expression.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/uniform_buffer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/vertex_attribute.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/texture2d.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/renderer/change_request.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/renderer/layer_group.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/renderer/layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/renderer/render_target.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/shaders/shader_program_base.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/suppress_copies.hpp
)
list(APPEND SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/drawable.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/drawable_builder.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/drawable_builder_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/drawable_builder_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/drawable_atlases_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/drawable_custom_layer_host_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/dynamic_texture.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/dynamic_texture_atlas.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/gpu_expression.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/hillshade_prepare_drawable_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/color_relief_drawable_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/image_drawable_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/line_drawable_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/symbol_drawable_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/collision_drawable_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/uniform_buffer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/vertex_attribute.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_target.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/change_request.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layer_group.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/tile_layer_group.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/background_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/background_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/circle_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/circle_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/fill_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/fill_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/fill_extrusion_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/fill_extrusion_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/fill_extrusion_shadow_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/fill_extrusion_shadow_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/heatmap_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/heatmap_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/heatmap_texture_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/heatmap_texture_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/hillshade_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/hillshade_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/hillshade_prepare_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/hillshade_prepare_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/line_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/line_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/location_indicator_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/location_indicator_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/raster_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/raster_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/symbol_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/symbol_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/collision_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/collision_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/shaders/shader_program_base.cpp
)

list(APPEND INCLUDE_FILES
    ${PROJECT_SOURCE_DIR}/include/mln/actor/actor_ref.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/actor/actor.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/actor/aspiring_actor.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/actor/established_actor.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/actor/mailbox.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/actor/message.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/actor/optional_actor.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/actor/optional_actor_ref.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/actor/scheduler.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/annotation/annotation.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/backend_scope.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/backend.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/color_mode.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/command_encoder.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/debug_group.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/depth_mode.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/draw_mode.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/draw_scope.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/fill_generator.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/gfx_types.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/polyline_generator.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/renderable.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/renderbuffer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/renderer_backend.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/rendering_stats.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/shader_group.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/shader_registry.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/shader.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/stencil_mode.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/types.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/gfx/vertex_buffer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/background_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/circle_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/custom_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/fill_extrusion_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/fill_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/heatmap_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/hillshade_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/color_relief_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/layer_manager.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/line_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/location_indicator_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/raster_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/layermanager/symbol_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/map/bound_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/map/camera.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/map/change.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/map/map_observer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/map/map_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/map/map_projection.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/map/map.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/map/mode.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/map/projection_mode.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/math/angles.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/math/clamp.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/math/log2.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/math/minmax.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/math/wrap.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/platform/settings.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/platform/thread.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/renderer/query.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/renderer/renderer_frontend.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/renderer/renderer_observer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/renderer/renderer_state.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/renderer/renderer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/shaders/program_parameters.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/shaders/shader_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/database_file_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/file_source_manager.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/file_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/network_status.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/offline.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/online_file_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/resource_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/resource_transform.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/resource.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/storage/response.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/color_ramp_property_value.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/color_ramp_property_value.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/constant.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/coordinate.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/custom_geometry_source_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/filter.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/function.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/geojson_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/geojson.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/get_json_type.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/light.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/position.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/property_value.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/rotation.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/source_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/tileset.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/conversion/transition_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/assertion.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/at.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/boolean_operator.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/case.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/check_subtype.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/coalesce.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/coercion.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/collator_expression.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/collator.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/comparison.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/compound_expression.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/distance.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/dsl.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/error.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/expression.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/find_zoom_curve.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/format_expression.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/format_section_override.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/formatted.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/get_covering_stops.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/image_expression.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/image.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/in.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/index_of.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/interpolate.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/interpolator.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/is_constant.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/is_expression.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/length.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/let.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/literal.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/match.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/number_format.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/parsing_context.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/slice.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/step.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/type.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/value.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/expression/within.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/filter.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/image.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/background_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/circle_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/custom_layer_host.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/custom_layer_init_parameters.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/custom_layer_render_parameters.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/custom_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/fill_extrusion_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/fill_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/heatmap_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/hillshade_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/color_relief_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/line_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/location_indicator_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/raster_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/layers/symbol_layer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/light.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/position.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/property_expression.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/property_value.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/rotation.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/sources/custom_geometry_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/sources/custom_vector_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/sources/geojson_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/sources/image_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/sources/raster_dem_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/sources/raster_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/sources/tile_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/sources/vector_source.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/sprite.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/style_property.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/style.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/transition_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/types.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/undefined.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/style/variable_anchor_offset_collection.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/text/glyph_range.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/text/glyph.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/tile/tile_id.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/tile/tile_necessity.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/tile/tile_operation.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/action_journal_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/action_journal.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/async_request.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/async_task.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/char_array_buffer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/chrono.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/client_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/color.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/compression.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/constants.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/containers.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/convert.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/default_style.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/enum.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/event.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/exception.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/expected.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/feature.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/font_stack.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/geo.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/geojson.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/geometry.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/hash.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/identity.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/ignore.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/image.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/immutable.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/indexed_tuple.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/instrumentation.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/interpolate.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/logging.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/lru_cache.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/noncopyable.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/padding.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/vertical_gradient.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/platform.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/premultiply.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/projection.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/range.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/rect.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/run_loop.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/scoped.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/size.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/string_indexer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/string.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/symbol_error_observer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/thread.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/tile_server_options.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/tileset.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/timer.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/tiny_unordered_map.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/traits.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/type_list.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/unitbezier.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/util.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/variant.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/vectors.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/work_request.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/work_task_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/mln/util/work_task.hpp
)
list(APPEND SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/mln/actor/mailbox.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/actor/scheduler.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/algorithm/update_renderables.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/algorithm/update_tile_masks.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/annotation_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/annotation_manager.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/annotation_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/annotation_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/annotation_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/annotation_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/fill_annotation_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/fill_annotation_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/line_annotation_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/line_annotation_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/render_annotation_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/render_annotation_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/shape_annotation_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/shape_annotation_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/symbol_annotation_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/annotation/symbol_annotation_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/geometry/anchor.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/geometry/debug_font_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/geometry/dem_data.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/geometry/dem_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/geometry/feature_index.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/geometry/feature_index.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/geometry/line_atlas.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/geometry/line_atlas.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/attribute.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/attribute.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/cull_face_mode.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/index_buffer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/index_vector.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/offscreen_texture.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/polyline_generator.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/fill_generator.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/render_pass.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/renderer_backend.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/rendering_stats.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/shader_group.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/shader_registry.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/uniform.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/upload_pass.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/gfx/vertex_vector.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/background_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/circle_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/fill_extrusion_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/fill_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/heatmap_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/hillshade_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/color_relief_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/layer_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/line_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/raster_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/symbol_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/location_indicator_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_location_indicator_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_location_indicator_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/location_indicator_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/location_indicator_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/location_indicator_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/location_indicator_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/location_indicator_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/circle_layout.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/clip_lines.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/clip_lines.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/layout.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/merge_lines.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/merge_lines.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/pattern_layout.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/symbol_feature.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/symbol_instance.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/symbol_instance.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/symbol_layout.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/symbol_layout.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/symbol_projection.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layout/symbol_projection.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/map.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/map_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/map_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/map_options.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/map_projection.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/transform.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/transform.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/transform_state.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/transform_state.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/map/zoom_history.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/math/log2.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/platform/settings.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/backend_scope.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/bucket_parameters.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/bucket_parameters.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/circle_bucket.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/circle_bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/debug_bucket.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/debug_bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/fill_bucket.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/fill_bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/fill_extrusion_bucket.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/fill_extrusion_bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/heatmap_bucket.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/heatmap_bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/hillshade_bucket.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/hillshade_bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/line_bucket.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/line_bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/raster_bucket.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/raster_bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/symbol_bucket.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/buckets/symbol_bucket.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/cross_faded_property_evaluator.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/cross_faded_property_evaluator.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/data_driven_property_evaluator.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/group_by_layout.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/group_by_layout.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/image_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/image_manager.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/image_manager_observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_background_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_background_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_circle_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_circle_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_fill_extrusion_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_fill_extrusion_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_fill_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_fill_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_heatmap_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_heatmap_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_hillshade_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_hillshade_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_color_relief_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_color_relief_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/color_relief_layer_tweaker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/color_relief_layer_tweaker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_line_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_line_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_raster_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_raster_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_symbol_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_symbol_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/paint_parameters.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/paint_parameters.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/paint_property_binder.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/paint_property_statistics.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/pattern_atlas.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/pattern_atlas.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/possibly_evaluated_property_value.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/property_evaluation_parameters.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/property_evaluator.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_light.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_light.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_orchestrator.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_orchestrator.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_pass.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_source_observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_static_data.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_static_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/render_tree.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/renderer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/renderer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/renderer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/renderer_state.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_custom_geometry_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_custom_geometry_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_custom_vector_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_custom_vector_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_geojson_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_geojson_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_image_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_image_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_raster_dem_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_raster_dem_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_raster_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_raster_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_tile_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_tile_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_vector_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/sources/render_vector_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/source_state.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/source_state.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/style_diff.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/style_diff.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/tile_mask.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/tile_parameters.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/tile_pyramid.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/tile_pyramid.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/tile_render_data.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/tile_render_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/transition_parameters.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/update_parameters.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/upload_parameters.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/shaders/shader_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/shaders/attributes.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/shaders/program_parameters.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/shaders/segment.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/shaders/uniforms.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/sprite/sprite_loader.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/sprite/sprite_loader.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/sprite/sprite_loader_observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/sprite/sprite_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/sprite/sprite_parser.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/asset_file_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/mbtiles_file_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/file_source_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/http_file_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/local_file_source.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/main_resource_loader.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/network_status.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/resource.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/resource_options.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/resource_transform.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/storage/response.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/collection.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/color_ramp_property_value.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/constant.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/sprite.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/coordinate.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/custom_geometry_source_options.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/filter.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/function.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/geojson.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/geojson_options.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/get_json_type.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/json.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/light.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/position.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/property_value.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/hillshade_conversions.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/rotation.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/source_options.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/stringify.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/tileset.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/conversion/transition_options.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/custom_tile_loader.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/custom_tile_loader.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/custom_vector_tile_loader.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/custom_vector_tile_loader.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/assertion.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/at.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/boolean_operator.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/case.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/check_subtype.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/coalesce.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/coercion.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/collator.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/collator_expression.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/comparison.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/compound_expression.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/distance.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/dsl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/dsl_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/expression.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/find_zoom_curve.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/format_expression.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/formatted.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/get_covering_stops.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/image.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/image_expression.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/in.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/index_of.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/interpolate.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/is_constant.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/is_expression.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/length.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/let.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/literal.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/match.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/number_format.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/parsing_context.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/slice.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/step.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/utf8_op_helpers.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/utf8_op_helpers.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/util.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/util.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/value.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/expression/within.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/filter.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/image.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sprite.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/image_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/image_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layer_observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/background_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/background_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/background_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/background_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/background_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/circle_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/circle_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/circle_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/circle_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/circle_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_extrusion_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_extrusion_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_extrusion_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_extrusion_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_extrusion_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/fill_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/heatmap_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/heatmap_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/heatmap_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/heatmap_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/heatmap_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/hillshade_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/hillshade_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/hillshade_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/hillshade_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/hillshade_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/color_relief_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/color_relief_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/color_relief_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/color_relief_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/color_relief_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/line_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/line_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/line_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/line_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/line_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/raster_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/raster_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/raster_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/raster_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/raster_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/symbol_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/symbol_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/symbol_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/symbol_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/symbol_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layout_property.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/light.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/light_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/light_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/light_observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/paint_property.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/parser.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/parser.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/property_expression.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/rapidjson_conversion.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/source_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/source_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/source_observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/custom_geometry_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/custom_geometry_source_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/custom_geometry_source_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/custom_vector_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/custom_vector_source_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/custom_vector_source_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/geojson_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/geojson_source_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/geojson_source_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/image_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/image_source_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/image_source_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/raster_dem_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/tile_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/raster_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/tile_source_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/tile_source_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/sources/vector_source.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/style.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/style_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/style_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/types.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/variable_anchor_offset_collection.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/custom_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/custom_layer_render_parameters.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/layermanager/custom_layer_factory.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/custom_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/style/layers/custom_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_custom_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_custom_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/bidi.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/check_max_angle.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/check_max_angle.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/collision_feature.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/collision_feature.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/collision_index.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/collision_index.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/cross_tile_symbol_index.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/cross_tile_symbol_index.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/get_anchors.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/get_anchors.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/glyph.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/glyph_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/glyph_manager.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/glyph_manager_observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/glyph_pbf.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/glyph_pbf.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/language_tag.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/language_tag.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/local_glyph_rasterizer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/placement.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/placement.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/quads.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/quads.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/shaping.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/shaping.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/tagged_string.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/text/tagged_string.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/custom_geometry_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/custom_geometry_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/custom_vector_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/custom_vector_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/geojson_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/geojson_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/geojson_tile_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/geometry_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/geometry_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/geometry_tile_data.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/geometry_tile_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/geometry_tile_worker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/geometry_tile_worker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/raster_dem_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/raster_dem_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/raster_dem_tile_worker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/raster_dem_tile_worker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/raster_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/raster_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/raster_tile_worker.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/raster_tile_worker.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile_cache.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile_cache.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile_id_hash.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile_id_io.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile_loader.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile_loader_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile_loader_observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile_observer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/tile_operation.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_mlt_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_mlt_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_mlt_tile_data.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_mlt_tile_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_mvt_tile.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_mvt_tile.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_mvt_tile_data.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/tile/vector_mvt_tile_data.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/action_journal.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/action_journal_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/camera.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/bounding_volumes.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/bounding_volumes.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/chrono.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/client_options.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/color$<IF:$<BOOL:${MLN_USE_RUST}>,.rs.cpp,.cpp>
    ${PROJECT_SOURCE_DIR}/src/mln/util/constants.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/convert.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/error_sink.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/error_sink.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/event.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/filesystem.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/font_stack.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/geo.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/geojson_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/geometry_util.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/geometry_util.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/grid_index.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/grid_index.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/http_header.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/http_header.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/http_timeout.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/http_timeout.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/i18n.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/i18n.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/identity.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/instrumentation.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/interpolate.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/intersection_tests.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/intersection_tests.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/io.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/io.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/literal.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/logging.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/longest_common_subsequence.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/mapbox.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/mapbox.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/mat2.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/mat2.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/mat3.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/mat3.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/mat4.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/mat4.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/math.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/padding.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/vertical_gradient.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/premultiply.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/quaternion.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/rapidjson.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/rapidjson.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/std.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/stopwatch.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/stopwatch.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/string.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/string_indexer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/thread.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/thread_local.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/thread_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/thread_pool.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/tile_coordinate.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/tile_cover.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/tile_cover.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/tile_cover_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/tile_cover_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/tile_range.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/default_style.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/tile_server_options.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/tiny_sdf.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/tiny_sdf.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/token.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/url.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/url.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/utf.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/version.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/version.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/util/work_request.cpp
)
list(APPEND SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer_render.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer_properties.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer_impl.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer_factory.hpp
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer_render.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer_impl.cpp
    ${PROJECT_SOURCE_DIR}/src/mln/plugin/plugin_layer_factory.cpp
)

include(${PROJECT_SOURCE_DIR}/cmake/opengl.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/metal.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/vulkan.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/webgpu.cmake)

list(APPEND
    SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/mln/layermanager/custom_drawable_layer_factory.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_custom_drawable_layer.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/renderer/layers/render_custom_drawable_layer.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/style/layers/custom_drawable_layer_impl.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/style/layers/custom_drawable_layer_impl.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/style/layers/custom_drawable_layer.cpp
)

# Harfbuzz text shaping
set(SHAPE_TARGETS, "")
list(APPEND
    SRC_FILES
        # Base text shaping files, which are always included. They don't depend on HarfBuzz if MLN_TEXT_SHAPING_HARFBUZZ is OFF.
        ${PROJECT_SOURCE_DIR}/src/mln/text/harfbuzz.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/text/harfbuzz.hpp
)
if (MLN_TEXT_SHAPING_HARFBUZZ)
    message(STATUS "Configuring with HarfBuzz text shaping")
    list(APPEND
        SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/mln/text/harfbuzz_impl.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/text/harfbuzz_impl.hpp
        ${PROJECT_SOURCE_DIR}/src/mln/text/freetype.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/text/freetype.hpp
    )
    target_compile_definitions(
        mbgl-core
        PRIVATE MLN_TEXT_SHAPING_HARFBUZZ=1
    )
    include(${PROJECT_SOURCE_DIR}/vendor/freetype.cmake)
    include(${PROJECT_SOURCE_DIR}/vendor/harfbuzz.cmake)

    target_include_directories(
        mbgl-core SYSTEM
        PUBLIC
            ${PROJECT_SOURCE_DIR}/vendor/freetype/include
            ${PROJECT_SOURCE_DIR}/vendor/harfbuzz/src
    )

endif()

target_sources(
    mbgl-core PRIVATE
    ${INCLUDE_FILES}
    ${SRC_FILES}
)

if (MLN_WITH_CLANG_TIDY)
    set_target_properties(mbgl-core PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()

set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/utf.cpp
    PROPERTIES
    COMPILE_FLAGS
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wno-deprecated-declarations>
)

source_group(TREE ${PROJECT_SOURCE_DIR}/include FILES ${INCLUDE_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR}/src FILES ${SRC_FILES})

if(EXISTS ${PROJECT_SOURCE_DIR}/.git/HEAD)
    execute_process(
        COMMAND
            git
            rev-parse
            --short=8
            HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE MLN_VERSION_REV
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(MLN_VERSION_REV 00000000)
endif()

set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/src/mln/util/version.cpp
    PROPERTIES
    COMPILE_DEFINITIONS
    MBGL_VERSION_REV="${MLN_VERSION_REV}"
)

target_include_directories(
    mbgl-core
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_include_directories(
    mbgl-core
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

include(${PROJECT_SOURCE_DIR}/vendor/args.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/boost.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/csscolorparser.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/earcut.hpp.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/eternal.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/rapidjson.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/maplibre-native-base.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/parsedate.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/pmtiles.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/polylabel.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/protozero.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/tracy.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/unique_resource.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/unordered_dense.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/vector-tile.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/wagyu.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/metal-cpp.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/maplibre-tile-spec.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/supercluster.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/expected-lite.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/kdbush.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/filesystem.cmake)

if(MLN_USE_RUST)
include(${PROJECT_SOURCE_DIR}/rustutils/rustutils.cmake)
endif()

target_link_libraries(
    mbgl-core
    PRIVATE
        mbgl-vendor-kdbush
        MapLibreNative::Base::shelf-pack-cpp
        MapLibreNative::Base::geojson-vt-cpp
        MapLibreNative::Base::cheap-ruler-cpp
        mbgl-compiler-options
        mbgl-vendor-boost
        mbgl-vendor-earcut.hpp
        mbgl-vendor-eternal
        mbgl-vendor-parsedate
        mbgl-vendor-pmtiles
        mbgl-vendor-polylabel
        mbgl-vendor-protozero
        mbgl-vendor-supercluster
        mbgl-vendor-unique_resource
        mbgl-vendor-vector-tile
        mbgl-vendor-wagyu
        mlt-cpp
        $<$<BOOL:${MLN_WITH_METAL}>:mbgl-vendor-metal-cpp>
        $<IF:$<BOOL:${MLN_USE_RUST}>,mbgl-rustutils,mbgl-vendor-csscolorparser>
        $<$<BOOL:${MLN_TEXT_SHAPING_HARFBUZZ}>:mbgl-freetype>
        $<$<BOOL:${MLN_TEXT_SHAPING_HARFBUZZ}>:mbgl-harfbuzz>
    PUBLIC
        MapLibreNative::Base
        mbgl-vendor-expected-lite
        mbgl-vendor-rapidjson
        MapLibreNative::Base::geojson.hpp
        MapLibreNative::Base::geometry.hpp
        MapLibreNative::Base::variant
        $<$<BOOL:${MLN_USE_TRACY}>:TracyClient>
        unordered_dense
)

set(EXPORT_TARGETS
    mbgl-core
    maplibre-native-base
    maplibre-native-base-cheap-ruler-cpp
    mbgl-vendor-expected-lite
    mbgl-vendor-kdbush
    maplibre-native-base-geojson-vt-cpp
    maplibre-native-base-geojson.hpp
    maplibre-native-base-geometry.hpp
    maplibre-native-base-jni.hpp
    maplibre-native-base-pixelmatch-cpp
    maplibre-native-base-shelf-pack-cpp
    mbgl-vendor-supercluster
    maplibre-native-base-variant
    mbgl-compiler-options
    mbgl-vendor-boost
    mbgl-vendor-earcut.hpp
    mbgl-vendor-eternal
    mbgl-vendor-filesystem
    mbgl-vendor-metal-cpp
    mbgl-vendor-rapidjson
    mbgl-vendor-parsedate
    mbgl-vendor-pmtiles
    mbgl-vendor-polylabel
    mbgl-vendor-protozero
    mbgl-vendor-unique_resource
    mbgl-vendor-vector-tile
    mbgl-vendor-wagyu
    mlt-cpp
    unordered_dense
)

if(MLN_USE_RUST)
    list(APPEND EXPORT_TARGETS mbgl-rustutils rustutils)
else()
    list(APPEND EXPORT_TARGETS mbgl-vendor-csscolorparser)
endif()

if(MLN_TEXT_SHAPING_HARFBUZZ)
    list(APPEND EXPORT_TARGETS mbgl-freetype mbgl-harfbuzz)
endif()

if(TARGET mbgl-vendor-dawn)
    list(APPEND EXPORT_TARGETS mbgl-vendor-dawn)
endif()

if(TARGET mbgl-vendor-wgpu)
    list(APPEND EXPORT_TARGETS mbgl-vendor-wgpu)
endif()

# The Xcode generator with Dawn component targets (no monolithic library) breaks export()
# because dawn_native/dawn_proc aren't in an export set. Skip export in that case.
set(_can_export TRUE)
if(TARGET mbgl-vendor-dawn AND NOT TARGET dawn::webgpu_dawn AND NOT MLN_WEBGPU_EMDAWN)
    set(_can_export FALSE)
endif()
if(_can_export)
    export(TARGETS ${EXPORT_TARGETS} FILE MapboxCoreTargets.cmake)
endif()

if(MLN_WITH_VULKAN)
    include(${PROJECT_SOURCE_DIR}/vendor/vulkan.cmake)

    target_link_libraries(
        mbgl-core
        PUBLIC
            mbgl-vendor-vulkan-headers
            mbgl-vendor-VulkanMemoryAllocator
            glslang
            SPIRV
            glslang-default-resource-limits
    )
endif()

set_target_properties(
    mbgl-core
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "MapLibre Native"
        INTERFACE_MAPLIBRE_URL "https://github.com/maplibre/maplibre-native"
        INTERFACE_MAPLIBRE_AUTHOR "MapLibre"
        INTERFACE_MAPLIBRE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md
)

if(MLN_WITH_METAL AND MLN_CREATE_AUTORELEASEPOOL)
    # When using the Metal backend, if there is is no AutoreleasePool, it will result
    # in memory leaks when autoreleased objects are created.

    # Platforms such as iOS and GLFW use ARC with @autoreleasepool blocks, for these platforms
    # creating an AutoreleasePool is not necessary (and in fact will not work).
    # When using MapLibre Native with Metal on other platforms such as Node.js, you can pass
    # this variable to ensure an AutoReleasePool is created in the render loop.
    # See also:
    # https://developer.apple.com/documentation/foundation/nsautoreleasepool
    # https://github.com/maplibre/maplibre-native/issues/2928
    # vendor/metal-cpp/README.md 'AutoreleasePools and Objects'
    target_compile_definitions(
        mbgl-core
            PRIVATE MLN_CREATE_AUTORELEASEPOOL=1
    )
endif()

set_property(TARGET mbgl-core PROPERTY FOLDER MapLibre)

add_library(
    Mapbox::Map ALIAS mbgl-core
)

if(MLN_WITH_CORE_ONLY)
    return()
endif()

include(${PROJECT_SOURCE_DIR}/scripts/license.cmake)

if(MLN_WITH_QT)
    include(${PROJECT_SOURCE_DIR}/platform/qt/qt.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL Android)
    include(${PROJECT_SOURCE_DIR}/platform/android/android.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    include(${PROJECT_SOURCE_DIR}/platform/linux/linux.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    include(${PROJECT_SOURCE_DIR}/platform/darwin/darwin.cmake)
    include(${PROJECT_SOURCE_DIR}/platform/macos/macos.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL iOS)
    include(${PROJECT_SOURCE_DIR}/platform/darwin/darwin.cmake)
    include(${PROJECT_SOURCE_DIR}/platform/ios/ios.cmake)
elseif(WIN32)
    include(${PROJECT_SOURCE_DIR}/platform/windows/windows.cmake)
else()
    message(FATAL_ERROR "Unsupported target platform: " ${CMAKE_SYSTEM_NAME})
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/test)
add_subdirectory(${PROJECT_SOURCE_DIR}/benchmark)
add_subdirectory(${PROJECT_SOURCE_DIR}/render-test)

include(cmake/mbgl-core-deps.cmake)
