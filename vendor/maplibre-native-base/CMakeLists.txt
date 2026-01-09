if(TARGET maplibre-native-base)
    return()
endif()

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(MAPBOX_BASE LANGUAGES CXX C)

option(MAPBOX_BASE_BUILD_TESTING "Bypass project target check and enforce building tests" OFF)

include(CTest)

cmake_policy(SET CMP0063 NEW)

if(NOT CMAKE_VERSION VERSION_LESS 3.13.0)
    # Required to allow maplibre-native-base to depend on libraries coming from /deps
    cmake_policy(SET CMP0079 NEW)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/deps)

if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MAPBOX_BASE_BUILD_TESTING) AND BUILD_TESTING)
    add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()
