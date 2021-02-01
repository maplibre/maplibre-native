cmake_minimum_required(VERSION 3.19)

message(source-dir=${CMAKE_SOURCE_DIR} current-dir=${CMAKE_CURRENT_SOURCE_DIR} current-binary-dir=${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR}/mapbox-gl-native )

project (mbgl)