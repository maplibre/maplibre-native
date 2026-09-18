add_subdirectory(ngon-layer)
target_link_libraries(mln-ngon-layer PUBLIC mbgl-core)

add_executable(mln-ngon-tests EXCLUDE_FROM_ALL ngon-layer/tests/ngon.test.cpp)
target_link_libraries(mln-ngon-tests PRIVATE mln-ngon-layer)

add_executable(mln-plugin-render-tests EXCLUDE_FROM_ALL render_test.cpp)
target_link_libraries(mln-plugin-render-tests PRIVATE mln-ngon-layer mbgl-render-test)
