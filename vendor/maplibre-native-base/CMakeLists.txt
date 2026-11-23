if(TARGET maplibre-native-base)
    return()
endif()

cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(MAPBOX_BASE LANGUAGES CXX C)

include(CTest)

# The tests: by default as `BUILD_TESTING` says, when this is the top-level project, and not when
# it is included in another project, such as MapLibre Native, whose `BUILD_TESTING` is for its
# own tests.
block()
    if(PROJECT_IS_TOP_LEVEL)
        set(default ${BUILD_TESTING})
    else()
        set(default OFF)
    endif()
    option(MAPBOX_BASE_BUILD_TESTING "Build the tests" ${default})
endblock()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/deps)

if(MAPBOX_BASE_BUILD_TESTING)
    add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()
