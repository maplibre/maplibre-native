enable_testing()

# The render tests of a plugin: the runner of render_test.cpp with the plugin registered, run by ctest from the
# repository root. Without MANIFEST the tests are the repository's own, with the manifest of this build's renderer.
# LAYER_TYPE and IGNORE select them, see render_test.cpp.
add_custom_target(mln-plugin-render-tests)
function(mln_plugin_render_test name plugin header register)
    cmake_parse_arguments(ARG "" "MANIFEST;LAYER_TYPE;IGNORE" "" ${ARGN})
    add_executable(${name} EXCLUDE_FROM_ALL ${CMAKE_CURRENT_SOURCE_DIR}/render_test.cpp)
    target_compile_definitions(${name} PRIVATE MLN_PLUGIN_HEADER="${header}" MLN_PLUGIN_REGISTER=${register})
    target_link_libraries(${name} PRIVATE ${plugin} mbgl-render-test)
    add_dependencies(mln-plugin-render-tests ${name})

    set(manifest ${ARG_MANIFEST})
    if(NOT manifest)
        if(MLN_WITH_METAL)
            set(manifest metrics/ios-metal-render-test-runner-style.json)
        elseif(MLN_WITH_VULKAN)
            set(manifest metrics/linux-vulkan.json)
        else()
            set(manifest metrics/linux-opengl.json)
        endif()
    endif()
    set(options --manifestPath ${manifest})
    if(ARG_LAYER_TYPE)
        list(APPEND options --layer-type ${ARG_LAYER_TYPE})
    endif()
    if(ARG_IGNORE)
        list(APPEND options --ignore ${ARG_IGNORE})
    endif()
    add_test(NAME ${name} COMMAND ${name} ${options} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endfunction()

add_subdirectory(plugins/ngon-layer)
target_link_libraries(mln-ngon-layer PUBLIC mbgl-core)

add_executable(mln-ngon-tests EXCLUDE_FROM_ALL plugins/ngon-layer/tests/ngon.test.cpp)
target_link_libraries(mln-ngon-tests PRIVATE mln-ngon-layer)

mln_plugin_render_test(mln-ngon-render-test mln-ngon-layer ngon_layer.hpp mln_ngon_layer_register
    MANIFEST plugins/plugins/ngon-layer/render-tests/manifest.json)

add_subdirectory(plugins/circle-layer)
target_link_libraries(mln-circle-layer PUBLIC mbgl-core)

add_executable(mln-circle-tests EXCLUDE_FROM_ALL plugins/circle-layer/tests/circle.test.cpp)
target_link_libraries(mln-circle-tests PRIVATE mln-circle-layer)

mln_plugin_render_test(mln-circle-render-test mln-circle-layer circle_layer.hpp mln_circle_layer_register
    LAYER_TYPE circle IGNORE plugins/plugins/circle-layer/render-tests/ignores.json)

add_library(mln-fill-extrusion STATIC plugins/fill-extrusion/shared/cpp/fill_extrusion.cpp)
target_include_directories(mln-fill-extrusion PUBLIC plugins/fill-extrusion/shared/include)
target_include_directories(mln-fill-extrusion PRIVATE plugins/fill-extrusion/vendor)
target_link_libraries(mln-fill-extrusion PUBLIC mbgl-core)
if(MLN_WITH_METAL)
    target_compile_definitions(mln-fill-extrusion PRIVATE MLN_FILL_EXTRUSION_METAL=1)
endif()

if(MLN_WITH_VULKAN OR MLN_WITH_METAL)
    mln_plugin_render_test(mln-fill-extrusion-render-test mln-fill-extrusion fill_extrusion.hpp mln_fill_extrusion_register
        LAYER_TYPE fill-extrusion IGNORE plugins/plugins/fill-extrusion/render-tests/ignores.json)
endif()

add_executable(mln-plugin-api-tests EXCLUDE_FROM_ALL
    ../test/plugin/api.test.cpp ../test/plugin/registry.test.cpp
    ../test/plugin/layout.test.cpp ../test/plugin/property_binding.test.cpp ../test/plugin/rendering.test.cpp
    $<$<BOOL:${MLN_WITH_VULKAN}>:${CMAKE_CURRENT_SOURCE_DIR}/plugins/fill-extrusion/tests/host.test.cpp>
    plugins/fill-extrusion/tests/geometry.test.cpp
    ../test/src/mln/test/stub_file_source.cpp
    ../vendor/googletest/googletest/src/gtest_main.cc)
target_include_directories(mln-plugin-api-tests PRIVATE ../src ../test/include ../test/src)
get_target_property(MLN_PLUGIN_TEST_LIBRARIES mbgl-core LINK_LIBRARIES)
target_link_libraries(mln-plugin-api-tests PRIVATE mln-fill-extrusion mbgl-vendor-googletest ${MLN_PLUGIN_TEST_LIBRARIES})
add_test(NAME mln-plugin-api-tests COMMAND mln-plugin-api-tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
