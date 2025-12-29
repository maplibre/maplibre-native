if(NOT MLN_WITH_WEBGPU)
    return()
endif()
message(STATUS "Configuring WebGPU renderer backend")

target_compile_definitions(
        mbgl-core
        PUBLIC
        MLN_RENDER_BACKEND_WEBGPU=1
)

list(APPEND
        SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/buffer_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/context.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/command_encoder.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/headless_backend.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/renderer_backend.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/drawable.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/drawable_builder.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/draw_scope_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/uniform_buffer.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/shader_program.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/wgsl_preprocessor.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/background.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/circle.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/clipping_mask.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/collision.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/custom_geometry.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/custom_symbol_icon.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/debug.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/fill.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/fill_extrusion.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/heatmap.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/heatmap_texture.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/hillshade.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/hillshade_prepare.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/line.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/location_indicator.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/raster.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/symbol.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/widevector.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/vertex_buffer_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/vertex_attribute.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/texture2d.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/offscreen_texture.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/tile_layer_group.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/layer_group.cpp
)

list(APPEND
        INCLUDE_FILES
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/buffer_resource.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/command_encoder.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/context.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/drawable.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/drawable_builder.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/index_buffer_resource.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/render_pass.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/renderer_backend.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/shader_program.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/background.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/circle.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/clipping_mask.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/collision.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/common.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/custom_geometry.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/custom_symbol_icon.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/debug.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/fill.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/fill_extrusion.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/heatmap.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/heatmap_texture.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/hillshade.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/hillshade_prepare.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/line.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/location_indicator.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/raster.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/symbol.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/shaders/webgpu/widevector.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/texture2d.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/uniform_buffer.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/upload_pass.hpp
        ${PROJECT_SOURCE_DIR}/include/mbgl/webgpu/vertex_buffer_resource.hpp
)

list(APPEND
        SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/buffer_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/command_encoder.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/context.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/drawable.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/drawable_builder.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/index_buffer_resource.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/render_pass.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/renderer_backend.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/webgpu/shader_program.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/texture2d.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/uniform_buffer.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/upload_pass.cpp
        ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/vertex_buffer_resource.cpp
)

# Include WebGPU vendor configuration (Dawn or wgpu)
include(${PROJECT_SOURCE_DIR}/vendor/webgpu.cmake)

# Include Dawn integration when requested
if(MLN_WEBGPU_IMPL_DAWN)
    include(${PROJECT_SOURCE_DIR}/vendor/dawn.cmake)
    if(TARGET mbgl-vendor-dawn)
        target_link_libraries(mbgl-core PRIVATE mbgl-vendor-dawn)
    endif()
elseif(MLN_WEBGPU_IMPL_WGPU)
    # Include wgpu-native integration
    include(${PROJECT_SOURCE_DIR}/vendor/wgpu.cmake)
    if(TARGET mbgl-vendor-wgpu)
        target_link_libraries(mbgl-core PRIVATE mbgl-vendor-wgpu)
        # Add WebGPU-Cpp implementation file (required for wgpu-native backend)
        list(APPEND SRC_FILES ${PROJECT_SOURCE_DIR}/src/mbgl/webgpu/webgpu_cpp_impl.cpp)
    endif()
endif()
