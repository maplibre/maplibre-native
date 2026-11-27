if(TARGET maplibre-native-base)
    return()
endif()

cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(MAPBOX_BASE LANGUAGES CXX C)

include(CTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/deps)

if(MLN_WITH_TESTS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()
