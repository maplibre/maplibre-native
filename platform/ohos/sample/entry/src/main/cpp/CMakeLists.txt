cmake_minimum_required(VERSION 3.16)

project(MapLibreNativeOhosSample)

set(MAPLIBRE_NATIVE_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../../.."
    CACHE PATH
    "Path to the MapLibre Native checkout"
)
get_filename_component(MAPLIBRE_NATIVE_ROOT "${MAPLIBRE_NATIVE_ROOT}" ABSOLUTE)

set(MLN_WITH_GLFW OFF CACHE BOOL "" FORCE)
set(MLN_WITH_OPENGL ON CACHE BOOL "" FORCE)
set(MLN_WITH_EGL ON CACHE BOOL "" FORCE)
set(MLN_OHOS_BUILD_NATIVE_MODULE ON CACHE BOOL "" FORCE)
# net_http returns persistent OH_HTTP_OUT_OF_MEMORY on retail HarmonyOS; use HMS RCP.
set(MLN_OHOS_WITH_HMS_RCP ON CACHE BOOL "" FORCE)

add_subdirectory("${MAPLIBRE_NATIVE_ROOT}" "${CMAKE_BINARY_DIR}/maplibre-native")
