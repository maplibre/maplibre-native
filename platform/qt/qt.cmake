message(STATUS "Configuring GL-Native with Qt bindings")

file(READ "${PROJECT_SOURCE_DIR}/platform/qt/VERSION" MBGL_QT_VERSION)
string(REGEX REPLACE "\n" "" MBGL_QT_VERSION "${MBGL_QT_VERSION}") # get rid of the newline at the end
set(MBGL_QT_VERSION_COMPATIBILITY 2.0.0)
message(STATUS "Version ${MBGL_QT_VERSION}")

option(MBGL_QT_LIBRARY_ONLY "Build only libraries" OFF)
option(MBGL_QT_STATIC "Build MapLibre GL Qt bindings staticly" OFF)
option(MBGL_QT_INSIDE_PLUGIN "Build QMapLibreGL as OBJECT library, so it can be bundled into separate single plugin lib." OFF)
option(MBGL_QT_WITH_HEADLESS "Build MapLibre GL Qt with headless support" ON)
option(MBGL_QT_WITH_INTERNAL_SQLITE "Build MapLibre GL Qt bindings with internal sqlite" OFF)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}
             COMPONENTS Gui
                        Network
             REQUIRED)

if(NOT MBGL_QT_LIBRARY_ONLY)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
    if (Qt6_FOUND)
        find_package(Qt${QT_VERSION_MAJOR}OpenGLWidgets REQUIRED)
    endif()
endif()

if(NOT MBGL_QT_WITH_INTERNAL_SQLITE)
    find_package(Qt${QT_VERSION_MAJOR}Sql REQUIRED)
else()
    message(STATUS "Using internal sqlite")
    include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(MBGL_QT_WITH_INTERNAL_ICU "Build MapLibre GL Qt bindings with internal ICU" OFF)
    if(NOT MBGL_QT_WITH_INTERNAL_ICU)
       find_package(ICU COMPONENTS uc REQUIRED)
    else()
       message(STATUS "Using internal ICU")
       include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)
    endif()
endif()

if(MSVC)
    add_definitions("/DQT_COMPILING_QIMAGE_COMPAT_CPP")
    add_definitions("/D_USE_MATH_DEFINES")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions("-DQT_COMPILING_QIMAGE_COMPAT_CPP")
    add_definitions("-D_USE_MATH_DEFINES")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL iOS)
    option(MBGL_QT_WITH_IOS_CCACHE "Enable ccache for iOS" OFF)
    if(MBGL_QT_WITH_IOS_CCACHE)
        include(${PROJECT_SOURCE_DIR}/platform/ios/ccache.cmake)
    endif()
endif()

if(ANDROID)
    message(STATUS "Building for ABI: ${ANDROID_ABI}")
    set(CMAKE_STATIC_LIBRARY_SUFFIX "_${ANDROID_ABI}.a")
elseif(CMAKE_SYSTEM_NAME STREQUAL iOS)
    set(CMAKE_DEBUG_POSTFIX "_debug")
elseif(MSVC OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

target_sources(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/$<IF:$<PLATFORM_ID:Linux>,default/src/mbgl/text/bidi.cpp,qt/src/mbgl/bidi.cpp>
        ${PROJECT_SOURCE_DIR}/platform/default/include/mbgl/gfx/headless_backend.hpp
        ${PROJECT_SOURCE_DIR}/platform/default/include/mbgl/gfx/headless_frontend.hpp
        ${PROJECT_SOURCE_DIR}/platform/default/include/mbgl/gl/headless_backend.hpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_backend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_frontend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gl/headless_backend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/i18n/collator.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/layermanager/layer_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/platform/time.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/asset_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/database_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/file_source_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/file_source_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/local_file_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/local_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/main_resource_loader.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/mbtiles_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/offline.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/offline_database.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/offline_download.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/online_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/$<IF:$<BOOL:${MBGL_QT_WITH_INTERNAL_SQLITE}>,default/src/mbgl/storage/sqlite3.cpp,qt/src/mbgl/sqlite3.cpp>
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/compression.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/monotonic_timer.cpp
        $<$<BOOL:${MBGL_QT_WITH_HEADLESS}>:${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/headless_backend_qt.cpp>
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/async_task.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/async_task_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/gl_functions.cpp
        $<$<BOOL:${MBGL_PUBLIC_BUILD}>:${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/http_file_source.cpp>
        $<$<BOOL:${MBGL_PUBLIC_BUILD}>:${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/http_file_source.hpp>
        $<$<BOOL:${MBGL_PUBLIC_BUILD}>:${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/http_request.cpp>
        $<$<BOOL:${MBGL_PUBLIC_BUILD}>:${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/http_request.hpp>
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/image.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/number_format.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/local_glyph_rasterizer.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/logging_qt.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/run_loop.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/run_loop_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/string_stdlib.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/thread.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/thread_local.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/timer_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/utf.cpp
)

target_compile_definitions(
    mbgl-core
    PRIVATE QT_IMAGE_DECODERS
    PUBLIC __QT__ MBGL_USE_GLES2
)

target_include_directories(
    mbgl-core
    PRIVATE ${PROJECT_SOURCE_DIR}/platform/default/include
)

include(GNUInstallDirs)
include(${PROJECT_SOURCE_DIR}/vendor/nunicode.cmake)

set_property(TARGET mbgl-core PROPERTY AUTOMOC ON)

target_link_libraries(
    mbgl-core
    PRIVATE
        $<$<NOT:$<OR:$<PLATFORM_ID:Windows>,$<PLATFORM_ID:Emscripten>>>:z>
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Network
        $<IF:$<BOOL:${MBGL_QT_WITH_INTERNAL_SQLITE}>,mbgl-vendor-sqlite,Qt${QT_VERSION_MAJOR}::Sql>
        $<$<PLATFORM_ID:Linux>:$<IF:$<BOOL:${MBGL_QT_WITH_INTERNAL_ICU}>,mbgl-vendor-icu,ICU::uc>>
        mbgl-vendor-nunicode
)

set(qmaplibregl_headers
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/QMapLibreGL
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/export.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/map.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/Map
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/settings.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/Settings
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/types.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/Types
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/utils.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapLibreGL/Utils
)

if (MBGL_QT_INSIDE_PLUGIN)
    add_library(qmaplibregl OBJECT)
elseif(MBGL_QT_STATIC)
    add_library(qmaplibregl STATIC)
else()
    add_library(qmaplibregl SHARED)
endif()

target_sources(
    qmaplibregl
    PRIVATE
    ${qmaplibregl_headers}
    ${PROJECT_SOURCE_DIR}/platform/qt/src/map.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/map_p.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/settings.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/types.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/map_observer.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/map_observer.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/map_renderer.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/map_renderer.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/renderer_backend.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/renderer_backend.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/renderer_observer.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/scheduler.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/scheduler.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/conversion.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/geojson.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/geojson.hpp
)

# Linux/Mac: Set framework, version and headers
set_target_properties(
    qmaplibregl PROPERTIES
    AUTOMOC ON
    EXPORT_NAME QMapLibreGL
    OUTPUT_NAME QMapLibreGL
    VERSION ${MBGL_QT_VERSION}
    SOVERSION ${MBGL_QT_VERSION_COMPATIBILITY}
    PUBLIC_HEADER "${qmaplibregl_headers}"
)
if (APPLE AND NOT MBGL_QT_STATIC AND NOT MBGL_QT_INSIDE_PLUGIN)
    set_target_properties(
        qmaplibregl PROPERTIES
        FRAMEWORK ON
        FRAMEWORK_VERSION A
        MACOSX_FRAMEWORK_IDENTIFIER org.maplibre.QMapLibreGL
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${MBGL_QT_VERSION}
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${MBGL_QT_VERSION}
    )
    target_include_directories(
        qmaplibregl
        INTERFACE
            $<INSTALL_INTERFACE:lib/QMapLibreGL.framework>
    )
endif()

include(CMakePackageConfigHelpers)
set(CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/QMapLibreGL/)

configure_package_config_file(
    "platform/qt/QMapLibreGLConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/QMapLibreGLConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_PREFIX CMAKE_INSTALL_INCLUDEDIR
    CMAKE_INSTALL_LIBDIR NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/QMapLibreGLConfigVersion.cmake
    VERSION ${MBGL_QT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(EXPORT QMapLibreGLTargets
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT development)

export(EXPORT QMapLibreGLTargets)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/QMapLibreGLConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/QMapLibreGLConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT development)

install(
    DIRECTORY include/mbgl
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT development
)

# FIXME: Because of rapidjson conversion
target_include_directories(
    qmaplibregl
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/platform/qt/include
)

target_compile_definitions(
    qmaplibregl
    PRIVATE
    QT_BUILD_MAPLIBREGL_LIB
)

target_link_libraries(
    qmaplibregl
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Network
    PRIVATE
        $<BUILD_INTERFACE:mbgl-compiler-options>
        $<BUILD_INTERFACE:mbgl-core>
        $<BUILD_INTERFACE:mbgl-vendor-parsedate>
        $<BUILD_INTERFACE:mbgl-vendor-nunicode>
        $<BUILD_INTERFACE:mbgl-vendor-csscolorparser>
)
# Do not use generator expressions for cleaner output
if (MBGL_QT_STATIC AND NOT MBGL_QT_INSIDE_PLUGIN)
    target_link_libraries(
        qmaplibregl
        PUBLIC
            $<$<NOT:$<BOOL:${MBGL_QT_WITH_INTERNAL_SQLITE}>>:Qt${QT_VERSION_MAJOR}::Sql>
            $<$<NOT:$<OR:$<PLATFORM_ID:Windows>,$<PLATFORM_ID:Emscripten>>>:z>
    )
endif()

if (MBGL_QT_STATIC OR MBGL_QT_INSIDE_PLUGIN)
    # Don't add import/export into public header because we don't build shared library.
    # In case on MBGL_QT_INSIDE_PLUGIN it's always OBJECT library and bundled into one
    # single Qt plugin lib.
    target_compile_definitions(
        qmaplibregl
        PUBLIC QT_MAPLIBREGL_STATIC
    )
endif()


install(TARGETS qmaplibregl
        EXPORT QMapLibreGLTargets
        # Explicit set of DESTINATION is needed for older CMake versions.
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        FRAMEWORK DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QMapLibreGL"
)

if(NOT MBGL_QT_LIBRARY_ONLY)
    add_executable(
        mbgl-qt
        ${PROJECT_SOURCE_DIR}/platform/qt/app/main.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/app/mapwindow.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/app/mapwindow.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/resources/common.qrc
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        set(CMAKE_EXECUTABLE_SUFFIX ".html")
    endif()

    # Qt public API should keep compatibility with old compilers for legacy systems
    set_property(TARGET mbgl-qt PROPERTY CXX_STANDARD 98)
    set_property(TARGET mbgl-qt PROPERTY AUTOMOC ON)

    target_link_libraries(
        mbgl-qt
        PRIVATE
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Gui
            $<$<BOOL:${Qt6_FOUND}>:Qt${QT_VERSION_MAJOR}::OpenGLWidgets>
            mbgl-compiler-options
            qmaplibregl
    )

    target_include_directories(
        mbgl-qt
        PRIVATE ${PROJECT_SOURCE_DIR}/platform/qt/include
    )

    add_executable(
        mbgl-test-runner
        ${PROJECT_SOURCE_DIR}/platform/qt/test/main.cpp
    )

    target_include_directories(
        mbgl-test-runner
        PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/test/include
    )

    target_compile_definitions(
        mbgl-test-runner
        PRIVATE WORK_DIRECTORY=${PROJECT_SOURCE_DIR}
    )

    target_link_libraries(
        mbgl-test-runner
        PRIVATE
            Qt${QT_VERSION_MAJOR}::Gui
            mbgl-compiler-options
            $<$<NOT:$<BOOL:MSVC>>:pthread>
    )

    if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
        target_link_libraries(
            mbgl-test-runner
            PRIVATE -Wl,-force_load mbgl-test
        )
    elseif(MSVC)
        target_link_options(
            mbgl-test-runner
            PRIVATE /WHOLEARCHIVE:mbgl-test.lib
        )
        target_link_libraries(
            mbgl-test-runner
            PRIVATE mbgl-test
        )
    else()
        target_link_libraries(
            mbgl-test-runner
            PRIVATE -Wl,--whole-archive mbgl-test -Wl,--no-whole-archive
        )
    endif()
endif()

find_program(MBGL_QDOC NAMES qdoc)

if(MBGL_QDOC)
    add_custom_target(mbgl-qt-docs)

    add_custom_command(
        TARGET mbgl-qt-docs PRE_BUILD
        COMMAND
            ${MBGL_QDOC}
            ${PROJECT_SOURCE_DIR}/platform/qt/config.qdocconf
            -outputdir
            ${CMAKE_BINARY_DIR}/docs
    )
endif()

if(NOT MBGL_QT_LIBRARY_ONLY)
    add_test(NAME mbgl-test-runner COMMAND mbgl-test-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
