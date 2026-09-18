find_program(MLN_NODE_EXECUTABLE NAMES node REQUIRED)
set(ngon_shader_header "${CMAKE_CURRENT_BINARY_DIR}/generated/ngon_shader_sources.hpp")
add_custom_command(
    OUTPUT "${ngon_shader_header}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND "${MLN_NODE_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate-shaders.mjs"
        --output "${ngon_shader_header}"
    DEPENDS scripts/generate-shaders.mjs
    COMMENT "Generating n-gon shaders"
    VERBATIM
)

add_library(mln-ngon-layer STATIC shared/cpp/ngon_layer.cpp "${ngon_shader_header}")
target_compile_features(mln-ngon-layer PUBLIC cxx_std_20)
set_target_properties(mln-ngon-layer PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(mln-ngon-layer PUBLIC shared/include)
target_include_directories(mln-ngon-layer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
# The parent supplies the plugin API headers through its SDK target.
