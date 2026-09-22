find_program(MLN_NODE_EXECUTABLE NAMES node REQUIRED)
set(ngon_shader_header "${CMAKE_CURRENT_BINARY_DIR}/generated/ngon_shader_sources.hpp")
add_custom_command(
    OUTPUT "${ngon_shader_header}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND "${MLN_NODE_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/ngon-layer/scripts/generate-shaders.mjs"
        --output "${ngon_shader_header}"
    DEPENDS ngon-layer/scripts/generate-shaders.mjs
    COMMENT "Generating n-gon shaders"
    VERBATIM
)

add_library(mln-ngon-layer STATIC ngon-layer/shared/cpp/ngon_layer.cpp "${ngon_shader_header}")
target_include_directories(mln-ngon-layer PUBLIC ngon-layer/shared/include)
target_include_directories(mln-ngon-layer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(mln-ngon-layer PUBLIC mbgl-core)

add_executable(mln-ngon-tests EXCLUDE_FROM_ALL ngon-layer/tests/ngon.test.cpp)
target_link_libraries(mln-ngon-tests PRIVATE mln-ngon-layer)

add_executable(mln-plugin-render-tests EXCLUDE_FROM_ALL render_test.cpp)
target_link_libraries(mln-plugin-render-tests PRIVATE mln-ngon-layer mbgl-render-test)

add_library(mln-fill-extrusion-plugin STATIC fill-extrusion-plugin/shared/cpp/fill_extrusion_plugin.cpp)
target_include_directories(mln-fill-extrusion-plugin PUBLIC fill-extrusion-plugin/shared/include)
target_link_libraries(mln-fill-extrusion-plugin PUBLIC mbgl-core)

add_executable(mln-fill-extrusion-plugin-tests EXCLUDE_FROM_ALL fill-extrusion-plugin/tests/fill_extrusion_plugin.test.cpp)
target_link_libraries(mln-fill-extrusion-plugin-tests PRIVATE mln-fill-extrusion-plugin)
