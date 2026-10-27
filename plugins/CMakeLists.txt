add_subdirectory(plugins/ngon-layer)
target_link_libraries(mln-ngon-layer PUBLIC mbgl-core)

add_executable(mln-ngon-tests EXCLUDE_FROM_ALL plugins/ngon-layer/tests/ngon.test.cpp)
target_link_libraries(mln-ngon-tests PRIVATE mln-ngon-layer)

add_executable(mln-plugin-render-tests EXCLUDE_FROM_ALL render_test.cpp)
target_link_libraries(mln-plugin-render-tests PRIVATE mln-ngon-layer mbgl-render-test)

add_library(mln-fill-extrusion STATIC plugins/fill-extrusion/shared/cpp/fill_extrusion.cpp)
target_include_directories(mln-fill-extrusion PUBLIC plugins/fill-extrusion/shared/include)
target_include_directories(mln-fill-extrusion PRIVATE plugins/fill-extrusion/vendor)
target_link_libraries(mln-fill-extrusion PUBLIC mbgl-core)
if(MLN_WITH_METAL)
    target_compile_definitions(mln-fill-extrusion PRIVATE MLN_FILL_EXTRUSION_METAL=1)
endif()

add_executable(mln-fill-extrusion-render-tests EXCLUDE_FROM_ALL plugins/fill-extrusion/render_test.cpp)
target_link_libraries(mln-fill-extrusion-render-tests PRIVATE mln-fill-extrusion mbgl-render-test)

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

if(MLN_WITH_METAL)
    # The glTF layer only has a Metal shader. tinygltf is fetched rather than vendored, and only its two source files
    # are used: its own CMake project also builds tests and install rules.
    include(FetchContent)
    FetchContent_Declare(
        tinygltf
        URL https://github.com/syoyo/tinygltf/archive/v3.0.1.tar.gz
        URL_HASH SHA256=b7e953f13a30d7b6fd677e484de35febde954143a265da15dacd92ed171c73e6
        SOURCE_SUBDIR none
    )
    FetchContent_MakeAvailable(tinygltf)

    add_library(mln-gltf-layer STATIC
        plugins/gltf-layer/shared/cpp/gltf_layer.cpp ${tinygltf_SOURCE_DIR}/tiny_gltf_v3.c)
    target_include_directories(mln-gltf-layer PUBLIC plugins/gltf-layer/shared/include ${PROJECT_SOURCE_DIR}/include)
    target_include_directories(mln-gltf-layer PRIVATE plugins/gltf-layer/shared/cpp ${tinygltf_SOURCE_DIR})

    add_executable(mln-gltf-tests EXCLUDE_FROM_ALL plugins/gltf-layer/tests/gltf.test.cpp)
    target_link_libraries(mln-gltf-tests PRIVATE mln-gltf-layer)
endif()
