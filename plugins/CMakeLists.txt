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

set(circle_shader_header "${CMAKE_CURRENT_BINARY_DIR}/generated/circle_shader_sources.hpp")
add_custom_command(
    OUTPUT "${circle_shader_header}"
    COMMAND "${MLN_NODE_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/circle-layer/scripts/generate-shaders.mjs"
        --output "${circle_shader_header}"
    DEPENDS circle-layer/scripts/generate-shaders.mjs
    VERBATIM
)
add_library(mln-circle-layer SHARED circle-layer/shared/cpp/circle_layer.cpp "${circle_shader_header}")
target_include_directories(mln-circle-layer PUBLIC circle-layer/shared/include "${PROJECT_SOURCE_DIR}/include")
target_include_directories(mln-circle-layer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_compile_features(mln-circle-layer PRIVATE cxx_std_20)
set_target_properties(mln-circle-layer PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)
add_executable(mln-circle-render-tests EXCLUDE_FROM_ALL circle-layer/render_test.cpp)
target_link_libraries(mln-circle-render-tests PRIVATE mln-circle-layer mbgl-render-test)
add_executable(mln-circle-tests EXCLUDE_FROM_ALL circle-layer/tests/circle.test.cpp)
target_link_libraries(mln-circle-tests PRIVATE mln-circle-layer)
