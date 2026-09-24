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

add_library(mln-fill-extrusion STATIC fill-extrusion/shared/cpp/fill_extrusion.cpp)
target_include_directories(mln-fill-extrusion PUBLIC fill-extrusion/shared/include)
target_include_directories(mln-fill-extrusion PRIVATE fill-extrusion/vendor)
target_link_libraries(mln-fill-extrusion PUBLIC mbgl-core)

add_executable(mln-fill-extrusion-render-tests EXCLUDE_FROM_ALL fill-extrusion/render_test.cpp)
target_link_libraries(mln-fill-extrusion-render-tests PRIVATE mln-fill-extrusion mbgl-render-test)

add_executable(mln-plugin-api-tests EXCLUDE_FROM_ALL
    ../test/plugin/api.test.cpp ../test/plugin/registry.test.cpp
    ../test/plugin/layout.test.cpp ../test/plugin/property_binding.test.cpp ../test/plugin/rendering.test.cpp
    $<$<BOOL:${MLN_WITH_VULKAN}>:${CMAKE_CURRENT_SOURCE_DIR}/fill-extrusion/tests/host.test.cpp>
    fill-extrusion/tests/geometry.test.cpp
    ../test/src/mln/test/stub_file_source.cpp
    ../vendor/googletest/googletest/src/gtest_main.cc)
target_include_directories(mln-plugin-api-tests PRIVATE ../src ../test/include ../test/src)
get_target_property(MLN_PLUGIN_TEST_LIBRARIES mbgl-core LINK_LIBRARIES)
target_link_libraries(mln-plugin-api-tests PRIVATE mln-fill-extrusion mbgl-vendor-googletest ${MLN_PLUGIN_TEST_LIBRARIES})

if(MLN_WITH_VULKAN)
    add_executable(mln-fill-extrusion-benchmark EXCLUDE_FROM_ALL
        fill-extrusion/benchmark.cpp ../test/src/mln/test/stub_file_source.cpp)
    target_include_directories(mln-fill-extrusion-benchmark PRIVATE ../src ../test/include ../test/src)
    target_link_libraries(mln-fill-extrusion-benchmark PRIVATE mln-fill-extrusion ${MLN_PLUGIN_TEST_LIBRARIES})
endif()
