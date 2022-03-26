cmake_minimum_required(VERSION 3.16)

file(READ "../qt/VERSION" MBGL_QML_VERSION)
string(REGEX REPLACE "\n" "" MBGL_QML_VERSION "${MBGL_QML_VERSION}") # get

project(MaplibreQmlPlugin
    VERSION "${MBGL_QML_VERSION}"
    DESCRIPTION "MapLibre Qt Location QML Plugin"
    LANGUAGES CXX C
)

# Make sure we use the fixed BASE argument of qt_add_resource.
set(QT_USE_FIXED_QT_ADD_RESOURCE_BASE TRUE)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core) # special case

if(WASM)
    message(NOTICE "Skipping the build as the condition \"NOT WASM\" is not met.")
    return()
endif()

include(QtCMakeVersionHelpers)
include(QtBuild)

# Need to search for positioning only after we make sure that it's not WASM.
# Otherwise we'll get an "QtPositioning not found" error in WASM build.
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Location Positioning Network) # special case
# find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Quick Qml Test QuickTest PositioningQuick ShaderTools) # special case

macro(assertTargets)
    foreach(qtTarget IN ITEMS ${ARGN})
        if(NOT TARGET Qt::${qtTarget})
           message(NOTICE "Skipping the build as the condition \"TARGET Qt::${qtTarget}\" is not met.")
           return()
        endif()
    endforeach()
endmacro()

set(MBGL_WITH_QT ON CACHE BOOL "Build Maplibre Qt version" FORCE)
set(MBGL_QT_STATIC  ON CACHE BOOL "Build as static library to be linked directly into Qt plugin lib" FORCE)
set(MBGL_QT_LIBRARY_ONLY ON CACHE BOOL "Build only the library part of Maplibre" FORCE)
set(MBGL_QT_INSIDE_QT_PLUGIN ON CACHE BOOL "Build all libs as OBJECT libraries." FORCE)
set(MBGL_ENGINE_PATH "../../" CACHE STRING "Maplibe Engine path" FORCE)
add_subdirectory(
    ${MBGL_ENGINE_PATH} # Source directory
    ${MBGL_ENGINE_PATH} # Binary directory
    EXCLUDE_FROM_ALL
)
if(IOS)
    target_compile_options(mbgl-core PUBLIC -fembed-bitcode)
    target_link_libraries(mbgl-core PUBLIC -fembed-bitcode)
    target_compile_options(mbgl-vendor-parsedate PUBLIC -fembed-bitcode)
    target_link_libraries(mbgl-vendor-parsedate PUBLIC -fembed-bitcode)
    target_compile_options(mbgl-vendor-nunicode PUBLIC -fembed-bitcode)
    target_link_libraries(mbgl-vendor-nunicode PUBLIC -fembed-bitcode)
    target_compile_options(mbgl-vendor-csscolorparser PUBLIC -fembed-bitcode)
    target_link_libraries(mbgl-vendor-csscolorparser PUBLIC -fembed-bitcode)
elseif(MSVC)
    target_compile_options(mbgl-core PUBLIC "/EHsc")
    target_compile_definitions(mbgl-core PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    target_compile_definitions(qmapboxgl PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif()


qt_internal_add_plugin(QGeoServiceProviderFactoryMaplibreGLPlugin
    OUTPUT_NAME qtgeoservices_maplibregl
    CLASS_NAME QGeoServiceProviderFactoryMaplibreGL
    PLUGIN_TYPE geoservices
    SOURCES
        qgeoserviceproviderpluginmaplibregl.cpp qgeoserviceproviderpluginmaplibregl.h
        qgeomappingmanagerenginemaplibregl.cpp qgeomappingmanagerenginemaplibregl.h
        qgeomapmaplibregl.cpp qgeomapmaplibregl.h qgeomapmaplibregl_p.h
        qmaplibreglstylechange.cpp qmaplibreglstylechange_p.h
        qsgmaplibreglnode.cpp qsgmaplibreglnode.h
    INCLUDE_DIRECTORIES
        ${MBGL_ENGINE_PATH}/platform/qt/include
    DEFINES
        QT_MAPBOXGL_STATIC
    LIBRARIES
        Qt::OpenGL
        Qt::Network
        Qt::LocationPrivate
        "$<BUILD_INTERFACE:qmapboxgl>"
        "$<BUILD_INTERFACE:mbgl-core>"
        "$<BUILD_INTERFACE:mbgl-vendor-parsedate>"
        "$<BUILD_INTERFACE:mbgl-vendor-nunicode>"
        "$<BUILD_INTERFACE:mbgl-vendor-csscolorparser>"
)

qt_add_resources(plugin_resource_files maplibregl.qrc)

qt_internal_extend_target(QGeoServiceProviderFactoryMaplibreGLPlugin
    SOURCES
        ${plugin_resource_files}
)
