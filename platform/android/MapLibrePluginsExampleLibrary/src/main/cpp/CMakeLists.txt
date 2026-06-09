# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name.
project("maplibrepluginsexamplelibrary")

# Create the plugin shared library
add_library(${CMAKE_PROJECT_NAME} SHARED
        maplibrepluginsexamplelibrary.cpp
        PluginLayerExampleAndroidRenderingCPP.cpp
        PluginLayerExampleAndroidRenderingCPP.h)

# Add rapidjson headers (header-only library)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../vendor/rapidjson/include)

# Add MapLibre headers
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # For <mbgl/...> public includes (e.g., mbgl/util/vectors.hpp)
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../include
        # For <mbgl/...> internal includes (e.g., mbgl/plugin/plugin_map_layer.hpp)
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../src
        # For <plugin/plugin_platform_android.hpp>
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../MapLibreAndroid/src/cpp)

# Find and link against libmaplibre.so from MapLibreAndroid build
# The library is built to: MapLibreAndroid/build/intermediates/cmake/{variant}/obj/{ABI}/libmaplibre.so
set(MAPLIBRE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../MapLibreAndroid/build/intermediates/cmake/openglDebug/obj/${ANDROID_ABI})

add_library(maplibre SHARED IMPORTED)
set_target_properties(maplibre PROPERTIES IMPORTED_LOCATION ${MAPLIBRE_LIB_DIR}/libmaplibre.so)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        maplibre
        GLESv3
        log)

