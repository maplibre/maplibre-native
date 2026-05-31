cmake_minimum_required(VERSION 3.16)

project(MapLibreNativeOhosSample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAPLIBRE_NATIVE_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../../.."
    CACHE PATH
    "Path to the MapLibre Native checkout"
)
get_filename_component(MAPLIBRE_NATIVE_ROOT "${MAPLIBRE_NATIVE_ROOT}" ABSOLUTE)

set(MLN_WITH_GLFW OFF CACHE BOOL "" FORCE)
set(MLN_OHOS_SAMPLE_RENDERER "Vulkan" CACHE STRING "Sample renderer backend: Vulkan or OpenGL")
set_property(CACHE MLN_OHOS_SAMPLE_RENDERER PROPERTY STRINGS Vulkan OpenGL)

if(MLN_OHOS_SAMPLE_RENDERER STREQUAL "Vulkan")
    set(MLN_WITH_VULKAN ON CACHE BOOL "" FORCE)
    set(MLN_WITH_OPENGL OFF CACHE BOOL "" FORCE)
    set(MLN_WITH_EGL OFF CACHE BOOL "" FORCE)
elseif(MLN_OHOS_SAMPLE_RENDERER STREQUAL "OpenGL")
    set(MLN_WITH_VULKAN OFF CACHE BOOL "" FORCE)
    set(MLN_WITH_OPENGL ON CACHE BOOL "" FORCE)
    set(MLN_WITH_EGL ON CACHE BOOL "" FORCE)
else()
    message(FATAL_ERROR "Unsupported MLN_OHOS_SAMPLE_RENDERER: ${MLN_OHOS_SAMPLE_RENDERER}")
endif()

add_subdirectory("${MAPLIBRE_NATIVE_ROOT}" "${CMAKE_BINARY_DIR}/maplibre-native")

if(MLN_WITH_VULKAN)
    set(MLN_OHOS_WINDOW_BACKEND_SOURCE ${MAPLIBRE_NATIVE_ROOT}/platform/ohos/src/vulkan_window_backend.cpp)
else()
    set(MLN_OHOS_WINDOW_BACKEND_SOURCE ${MAPLIBRE_NATIVE_ROOT}/platform/ohos/src/egl_window_backend.cpp)
endif()

add_library(
    maplibre_native_ohos
    SHARED
        ${MLN_OHOS_WINDOW_BACKEND_SOURCE}
        ${MAPLIBRE_NATIVE_ROOT}/platform/ohos/src/renderer_frontend.cpp
        gesture_handler.cpp
        map_view.cpp
        native_module.cpp
        native_values.cpp
)

set_target_properties(
    maplibre_native_ohos
    PROPERTIES
        OUTPUT_NAME maplibre_native_ohos
)

target_compile_definitions(
    maplibre_native_ohos
    PRIVATE
        OHOS_PLATFORM
)

target_compile_options(
    maplibre_native_ohos
    PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-command-line-argument>
        $<$<COMPILE_LANGUAGE:CXX>:-fexperimental-library>
)

target_include_directories(
    maplibre_native_ohos
    PRIVATE
        ${MAPLIBRE_NATIVE_ROOT}/src
        ${MAPLIBRE_NATIVE_ROOT}/platform/ohos/src
)

target_link_libraries(
    maplibre_native_ohos
    PRIVATE
        mbgl-compiler-options
        ace_napi.z
        ace_ndk.z
        hilog_ndk.z
        native_window
        $<$<BOOL:${MLN_WITH_OPENGL}>:EGL>
        $<$<BOOL:${MLN_WITH_OPENGL}>:GLESv3>
        $<$<BOOL:${MLN_WITH_VULKAN}>:vulkan>
)

mln_ohos_link_core_whole_archive(maplibre_native_ohos)
