cmake_minimum_required(VERSION 3.19)

project (mbgl)
enable_testing()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR}/mapbox-gl-native )
