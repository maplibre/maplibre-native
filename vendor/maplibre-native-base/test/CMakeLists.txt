# The tests of MapLibre Native Base, with the googletest and expected-lite
# MapLibre Native vendors.
include(${PROJECT_SOURCE_DIR}/../expected-lite.cmake)
include(${PROJECT_SOURCE_DIR}/../googletest.cmake)
find_package(Threads REQUIRED)

set(TEST_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_LIST_DIR}/test_defines.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/test_defines.hpp)

add_executable(
    maplibre-native-base-test
    ${CMAKE_CURRENT_LIST_DIR}/compatibility/value.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/io.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/io_delete.cpp
    ${CMAKE_CURRENT_LIST_DIR}/std/weak.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util/type_wrapper.cpp
    ${PROJECT_SOURCE_DIR}/../googletest/googletest/src/gtest_main.cc
)

target_include_directories(maplibre-native-base-test PRIVATE ${CMAKE_CURRENT_LIST_DIR}/io ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
    maplibre-native-base-test
    PRIVATE
        MapLibreNative::Base
        mbgl-vendor-expected-lite
        mbgl-vendor-googletest
        Threads::Threads
)

add_test(NAME maplibre-native-base-test COMMAND maplibre-native-base-test)
