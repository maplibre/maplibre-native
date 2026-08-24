if(NOT MLN_WITH_WEBGPU)
    return()
endif()
message(STATUS "Configuring WebGPU renderer backend")

target_compile_definitions(
        mbgl-core
        PUBLIC
        MLN_RENDER_BACKEND_WEBGPU=1
)

target_include_directories(
        mbgl-core
        PUBLIC
        ${PROJECT_SOURCE_DIR}/platform/default/include
)

list(APPEND
        SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/buffer_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/context.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/command_encoder.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/renderer_backend.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/drawable.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/drawable_builder.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/draw_scope_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/dynamic_texture.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/uniform_buffer.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/shader_program.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/wgsl_preprocessor.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/background.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/circle.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/clipping_mask.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/collision.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/custom_geometry.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/custom_symbol_icon.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/debug.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/fill.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/fill_extrusion.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/heatmap.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/heatmap_texture.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/hillshade.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/hillshade_prepare.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/color_relief.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/line.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/location_indicator.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/raster.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/symbol.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/widevector.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/vertex_buffer_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/vertex_attribute.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/texture2d.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/offscreen_texture.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/tile_layer_group.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/layer_group.cpp
)

list(APPEND
        INCLUDE_FILES
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/buffer_resource.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/command_encoder.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/context.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/drawable.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/drawable_builder.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/dynamic_texture.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/index_buffer_resource.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/render_pass.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/renderer_backend.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/shader_program.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/background.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/circle.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/clipping_mask.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/collision.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/common.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/custom_geometry.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/custom_symbol_icon.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/debug.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/fill.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/fill_extrusion.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/heatmap.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/heatmap_texture.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/hillshade.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/hillshade_prepare.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/color_relief.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/line.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/location_indicator.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/raster.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/symbol.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/shaders/webgpu/widevector.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/texture2d.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/uniform_buffer.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/upload_pass.hpp
        ${PROJECT_SOURCE_DIR}/include/mln/webgpu/vertex_buffer_resource.hpp
)

list(APPEND
        SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/buffer_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/command_encoder.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/context.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/drawable.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/drawable_builder.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/dynamic_texture.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/index_buffer_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/render_pass.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/renderer_backend.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/shaders/webgpu/shader_program.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/texture2d.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/uniform_buffer.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/upload_pass.cpp
        ${PROJECT_SOURCE_DIR}/src/mln/webgpu/vertex_buffer_resource.cpp
)

# Include WebGPU vendor configuration (Dawn or wgpu)
include(${PROJECT_SOURCE_DIR}/vendor/webgpu.cmake)

# Include Dawn integration when requested
if(MLN_WEBGPU_IMPL_DAWN)
    include(${PROJECT_SOURCE_DIR}/vendor/dawn.cmake)
    if(TARGET mbgl-vendor-dawn)
        if(MLN_WEBGPU_EMDAWN)
            # Emdawn is a compiler/linker port rather than a conventional
            # library. Consumers must inherit its final-link options.
            target_link_libraries(mbgl-core PUBLIC mbgl-vendor-dawn)
        else()
            target_link_libraries(mbgl-core PRIVATE mbgl-vendor-dawn)
        endif()
    endif()
elseif(MLN_WEBGPU_IMPL_WGPU OR MLN_WEBGPU_IMPL_FFI)
    # Include wgpu-native integration
    include(${PROJECT_SOURCE_DIR}/vendor/wgpu.cmake)
    if(TARGET mbgl-vendor-wgpu)
        target_link_libraries(mbgl-core PRIVATE mbgl-vendor-wgpu)
        # Add WebGPU-Cpp implementation file (required for wgpu-native backend)
        list(APPEND SRC_FILES ${PROJECT_SOURCE_DIR}/src/mln/webgpu/webgpu_cpp_impl.cpp)
    endif()
endif()

# Headless backend uses Dawn native / wgpu-native bootstrap; skip for emdawnwebgpu.
if(NOT MLN_WEBGPU_EMDAWN)
    list(APPEND SRC_FILES ${PROJECT_SOURCE_DIR}/src/mln/webgpu/headless_backend.cpp)
endif()
