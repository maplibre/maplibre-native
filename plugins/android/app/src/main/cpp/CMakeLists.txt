cmake_minimum_required(VERSION 3.24)
project(MapLibreNgonExample LANGUAGES CXX)

# Consume only the C header from the SDK AAR, never another copy of mbgl-core.
find_package(MapLibreAndroid REQUIRED CONFIG)
find_program(MLN_NODE_EXECUTABLE NAMES node REQUIRED)
get_filename_component(NGON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ngon-layer" ABSOLUTE)
set(ngon_shader_header "${CMAKE_CURRENT_BINARY_DIR}/generated/ngon_shader_sources.hpp")
add_custom_command(
    OUTPUT "${ngon_shader_header}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND "${MLN_NODE_EXECUTABLE}" "${NGON_ROOT}/scripts/generate-shaders.mjs" --output "${ngon_shader_header}"
    DEPENDS "${NGON_ROOT}/scripts/generate-shaders.mjs"
    VERBATIM
)

add_library(ngon-plugin SHARED
    plugin.cpp
    "${NGON_ROOT}/shared/cpp/ngon_layer.cpp"
    "${ngon_shader_header}"
)
target_compile_features(ngon-plugin PRIVATE cxx_std_20)
target_include_directories(ngon-plugin PRIVATE "${NGON_ROOT}/shared/include" "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(ngon-plugin PRIVATE MapLibreAndroid::maplibre dl)
