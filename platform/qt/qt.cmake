message(STATUS "Configuring GL-Native with Qt bindings")

# VLC-Qt version number
file(READ "${PROJECT_SOURCE_DIR}/platform/qt/VERSION" MBGL_QT_VERSION)
string(REGEX REPLACE "\n" "" MBGL_QT_VERSION "${MBGL_QT_VERSION}") # get rid of the newline at the end
set(MBGL_QT_VERSION_COMPATIBILITY 2.0.0)
message(STATUS "Version ${MBGL_QT_VERSION}")

option(MBGL_QT_LIBRARY_ONLY "Build only libraries" OFF)
option(MBGL_QT_STATIC "Build Mapbox GL Qt bindings staticly" OFF)
option(MBGL_QT_INSIDE_PLUGIN "Build qmapboxgl as OBJECT library, so it can be bundled into separate single plugin lib." OFF)
option(MBGL_WITH_QT_HEADLESS "Build Mapbox GL Qt with headless support" ON)
option(MBGL_QT_WITH_INTERNAL_SQLITE "Build Mapbox GL Qt bindings with internal sqlite" OFF)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}
             COMPONENTS Gui
                        Network
             REQUIRED)
if (Qt6_FOUND)
    find_package(Qt${QT_VERSION_MAJOR}OpenGLWidgets REQUIRED)
endif()

if(MBGL_WITH_QT_HEADLESS OR NOT MBGL_QT_LIBRARY_ONLY)
    find_package(Qt${QT_VERSION_MAJOR}
                 COMPONENTS OpenGL
                            Widgets
                 REQUIRED)
endif()

if(NOT MBGL_QT_WITH_INTERNAL_SQLITE)
    find_package(Qt${QT_VERSION_MAJOR}Sql REQUIRED)
else()
    message(STATUS "Using internal sqlite")
    include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(MBGL_QT_WITH_INTERNAL_ICU "Build Mapbox GL Qt bindings with internal ICU" OFF)
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
        ${PROJECT_SOURCE_DIR}/platform/$<IF:$<PLATFORM_ID:Linux>,default/src/mbgl/text/bidi.cpp,qt/src/bidi.cpp>
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
        ${PROJECT_SOURCE_DIR}/platform/$<IF:$<BOOL:${MBGL_QT_WITH_INTERNAL_SQLITE}>,default/src/mbgl/storage/sqlite3.cpp,qt/src/sqlite3.cpp>
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/compression.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/monotonic_timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/async_task.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/async_task_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/number_format.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/gl_functions.cpp
        $<$<BOOL:${MBGL_PUBLIC_BUILD}>:${PROJECT_SOURCE_DIR}/platform/qt/src/http_file_source.cpp>
        $<$<BOOL:${MBGL_PUBLIC_BUILD}>:${PROJECT_SOURCE_DIR}/platform/qt/src/http_file_source.hpp>
        $<$<BOOL:${MBGL_PUBLIC_BUILD}>:${PROJECT_SOURCE_DIR}/platform/qt/src/http_request.cpp>
        $<$<BOOL:${MBGL_PUBLIC_BUILD}>:${PROJECT_SOURCE_DIR}/platform/qt/src/http_request.hpp>
        ${PROJECT_SOURCE_DIR}/platform/qt/src/local_glyph_rasterizer.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/qt_image.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/qt_logging.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/run_loop.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/run_loop_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/string_stdlib.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/thread.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/thread_local.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/timer_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/utf.cpp
)

if(MBGL_WITH_QT_HEADLESS OR NOT MBGL_QT_LIBRARY_ONLY)
    target_sources(
        mbgl-core
        PRIVATE
           ${PROJECT_SOURCE_DIR}/platform/qt/src/headless_backend_qt.cpp
    )
    target_link_libraries(
        mbgl-core
        PRIVATE
	   Qt${QT_VERSION_MAJOR}::OpenGL
    )
endif()

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

set(qmapboxgl_headers
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapbox
    ${PROJECT_SOURCE_DIR}/platform/qt/include/QMapboxGL
    ${PROJECT_SOURCE_DIR}/platform/qt/include/qmapbox.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/include/qmapboxgl.hpp
)

if (MBGL_QT_INSIDE_PLUGIN)
    add_library(qmapboxgl OBJECT)
elseif(MBGL_QT_STATIC)
    add_library(qmapboxgl STATIC)
else()
    add_library(qmapboxgl SHARED)
endif()

target_sources(
    qmapboxgl
    PRIVATE
    ${qmapboxgl_headers}
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapbox.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl_map_observer.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl_map_observer.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl_map_renderer.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl_map_renderer.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl_p.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl_renderer_backend.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl_renderer_backend.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl_scheduler.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qmapboxgl_scheduler.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qt_conversion.hpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qt_geojson.cpp
    ${PROJECT_SOURCE_DIR}/platform/qt/src/qt_geojson.hpp
)

# Linux/Mac: Set framework, version and headers
set_target_properties(
    qmapboxgl PROPERTIES
    AUTOMOC ON
    EXPORT_NAME QMapboxGL
    OUTPUT_NAME QMapboxGL
    VERSION ${MBGL_QT_VERSION}
    SOVERSION ${MBGL_QT_VERSION_COMPATIBILITY}
    PUBLIC_HEADER "${qmapboxgl_headers}"
)
if (NOT MBGL_QT_STATIC AND NOT MBGL_QT_INSIDE_PLUGIN)
    set_target_properties(
        qmapboxgl PROPERTIES
        FRAMEWORK ON
        FRAMEWORK_VERSION A
    )
endif()

include(CMakePackageConfigHelpers)
set(CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/QMapboxGL/)

configure_package_config_file(
	"platform/qt/QMapboxGLConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/QMapboxGLConfig.cmake"
	INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
	PATH_VARS CMAKE_INSTALL_PREFIX CMAKE_INSTALL_INCLUDEDIR
	CMAKE_INSTALL_LIBDIR NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/QMapboxGLConfigVersion.cmake
	VERSION ${MBGL_QT_VERSION}
	COMPATIBILITY AnyNewerVersion)

install(EXPORT QMapboxGLTargets
	DESTINATION ${CMAKECONFIG_INSTALL_DIR}
	COMPONENT development)

export(EXPORT QMapboxGLTargets)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/QMapboxGLConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/QMapboxGLConfigVersion.cmake"
	DESTINATION ${CMAKECONFIG_INSTALL_DIR}
	COMPONENT development)

install(
    DIRECTORY include/mbgl
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT development
)

# FIXME: Because of rapidjson conversion
target_include_directories(
    qmapboxgl
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/platform/qt/include
    INTERFACE
    "$<INSTALL_INTERFACE:include>"
)

target_compile_definitions(
    qmapboxgl
    PRIVATE
    QT_BUILD_MAPBOXGL_LIB
)

target_link_libraries(
    qmapboxgl
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Network
    PRIVATE
        "$<BUILD_INTERFACE:mbgl-compiler-options>"
        "$<BUILD_INTERFACE:mbgl-core>"
        "$<BUILD_INTERFACE:mbgl-vendor-parsedate>"
        "$<BUILD_INTERFACE:mbgl-vendor-nunicode>"
        "$<BUILD_INTERFACE:mbgl-vendor-csscolorparser>"
)

install(TARGETS qmapboxgl
        EXPORT QMapboxGLTargets
        # Explicit set of DESTINATION is needed for older CMake versions.
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        FRAMEWORK DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
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

    target_compile_definitions(
        mbgl-qt
        PRIVATE
        $<$<OR:$<BOOL:${MBGL_QT_STATIC}>,$<BOOL:${MBGL_QT_INSIDE_PLUGIN}>>:QT_MAPBOXGL_STATIC>
    )

    target_link_libraries(
        mbgl-qt
        PRIVATE
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Gui
            $<$<BOOL:${Qt6_FOUND}>:Qt${QT_VERSION_MAJOR}::OpenGLWidgets>
            mbgl-compiler-options
            qmapboxgl
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
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Gui
            Qt${QT_VERSION_MAJOR}::OpenGL
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
