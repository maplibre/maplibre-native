add_library(mln-ngon-layer STATIC ngon-layer/shared/cpp/ngon_layer.cpp)
target_include_directories(mln-ngon-layer PUBLIC ngon-layer/shared/include)
target_link_libraries(mln-ngon-layer PUBLIC mbgl-core)

add_executable(mln-ngon-tests EXCLUDE_FROM_ALL ngon-layer/tests/ngon.test.cpp)
target_link_libraries(mln-ngon-tests PRIVATE mln-ngon-layer)

add_executable(mln-plugin-render-tests EXCLUDE_FROM_ALL render_test.cpp)
target_link_libraries(mln-plugin-render-tests PRIVATE mln-ngon-layer mbgl-render-test)

add_executable(mln-plugin-host-tests EXCLUDE_FROM_ALL
    tests/registry.test.cpp tests/property_binding.test.cpp)
target_link_libraries(mln-plugin-host-tests PRIVATE mln-ngon-layer mbgl-vendor-googletest mbgl-compiler-options)
target_include_directories(mln-plugin-host-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
