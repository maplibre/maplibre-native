if(TARGET maplibre-native-base)
    return()
endif()

cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(MAPBOX_BASE LANGUAGES CXX C)

# TODO just use MLN_WITH_TESTS, but we cannot do that yet because these tests have bit-rotted
option(MAPBOX_BASE_BUILD_TESTING "Build the tests" OFF)

include(CTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/deps)

if(MAPBOX_BASE_BUILD_TESTING)
    add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()
