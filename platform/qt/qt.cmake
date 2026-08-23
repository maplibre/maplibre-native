message(STATUS "Configuring MapLibre Native with Qt platform")

option(MLN_QT_LIBRARY_ONLY "Build only MapLibre Native Qt bindings libraries" OFF)
option(MLN_QT_WITH_INTERNAL_SQLITE "Build MapLibre Native Qt bindings with internal sqlite" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Threads REQUIRED)

    option(MLN_QT_WITH_INTERNAL_ICU "Build MapLibre GL Qt bindings with internal ICU" OFF)
    if(NOT MLN_QT_WITH_INTERNAL_ICU)
        # find ICU ignoring Qt paths
        option(MLN_QT_IGNORE_ICU "Ignore Qt-provided ICU library" ON)
        if(MLN_QT_IGNORE_ICU)
            set(_CMAKE_PREFIX_PATH_ORIG ${CMAKE_PREFIX_PATH})
            set(_CMAKE_FIND_ROOT_PATH_ORIG ${CMAKE_FIND_ROOT_PATH})
            unset(CMAKE_PREFIX_PATH)
            unset(CMAKE_FIND_ROOT_PATH)
        endif()

        # Use PkgConfig to find ICU
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(ICU_UC REQUIRED icu-uc)
        pkg_check_modules(ICU_I18N REQUIRED icu-i18n)

        if(MLN_QT_IGNORE_ICU)
            set(CMAKE_PREFIX_PATH ${_CMAKE_PREFIX_PATH_ORIG})
            set(CMAKE_FIND_ROOT_PATH ${_CMAKE_FIND_ROOT_PATH_ORIG})
            unset(_CMAKE_PREFIX_PATH_ORIG)
            unset(_CMAKE_FIND_ROOT_PATH_ORIG)
        endif()
    else()
        message(STATUS "Using internal ICU")
        include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)
    endif()
endif()

if("${QT_VERSION_MAJOR}" STREQUAL "")
    find_package(QT NAMES Qt6 COMPONENTS Core REQUIRED)
else()
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
endif()
find_package(Qt${QT_VERSION_MAJOR}
             COMPONENTS Gui
                        Network
             REQUIRED)

if(NOT MLN_QT_WITH_INTERNAL_SQLITE)
    find_package(Qt${QT_VERSION_MAJOR}Sql REQUIRED)
else()
    message(STATUS "Using internal sqlite")
    include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)
endif()

# Debugging & ccache on Windows
if (MSVC)
    foreach(config DEBUG RELWITHDEBINFO)
        foreach(lang C CXX)
            set(flags_var "CMAKE_${lang}_FLAGS_${config}")
            string(REPLACE "/Zi" "/Z7" ${flags_var} "${${flags_var}}")
            set(${flags_var} "${${flags_var}}" PARENT_SCOPE)
        endforeach()
    endforeach()
endif()

# Qt Vulkan renderer backend: only when MLN_WITH_VULKAN and qvulkaninstance.h present
if(MLN_WITH_VULKAN)
    find_path(QT_VULKAN_HEADER qvulkaninstance.h
        PATHS ${Qt${QT_VERSION_MAJOR}Gui_INCLUDE_DIRS}
        NO_DEFAULT_PATH)

    if(NOT QT_VULKAN_HEADER)
        message(FATAL_ERROR "Qt build has no Vulkan headers; can not build Qt Vulkan backend")
    endif()
endif()

target_sources(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/$<IF:$<PLATFORM_ID:Linux>,default/src/mln/text/bidi.cpp,qt/src/mln/bidi.cpp>
        ${PROJECT_SOURCE_DIR}/platform/default/include/mln/gfx/headless_backend.hpp
        ${PROJECT_SOURCE_DIR}/platform/default/include/mln/gfx/headless_frontend.hpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/gfx/headless_backend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/gfx/headless_frontend.cpp
        $<$<BOOL:${MLN_WITH_OPENGL}>:${PROJECT_SOURCE_DIR}/platform/default/src/mln/gl/headless_backend.cpp>
        $<$<BOOL:${MLN_WITH_METAL}>:${PROJECT_SOURCE_DIR}/platform/default/src/mln/mtl/headless_backend.cpp>
        $<$<BOOL:${MLN_WITH_VULKAN}>:${PROJECT_SOURCE_DIR}/platform/default/src/mln/vulkan/headless_backend.cpp>
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/i18n/collator.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/layermanager/layer_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/platform/time.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/asset_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/database_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/file_source_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/file_source_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/local_file_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/local_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/main_resource_loader.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/mbtiles_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/offline.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/offline_database.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/offline_download.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/online_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/$<IF:$<BOOL:${MLN_WITH_PMTILES}>,pmtiles_file_source.cpp,pmtiles_file_source_stub.cpp>
        ${PROJECT_SOURCE_DIR}/platform/$<IF:$<BOOL:${MLN_QT_WITH_INTERNAL_SQLITE}>,default/src/mln/storage/sqlite3.cpp,qt/src/mln/sqlite3.cpp>
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/compression.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/filesystem.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/monotonic_timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/async_task.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/async_task_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/gl_functions.cpp
        $<$<BOOL:${MLN_WITH_OPENGL}>:${PROJECT_SOURCE_DIR}/platform/qt/src/mln/headless_backend_qt.cpp>
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/http_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/http_file_source.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/http_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/http_request.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/image.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/number_format.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/local_glyph_rasterizer.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/logging_qt.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/run_loop.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/run_loop_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/string_stdlib.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/thread.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/thread_local.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/timer_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mln/utf.cpp
)

target_compile_definitions(
    mbgl-core
    PRIVATE
        QT_IMAGE_DECODERS
        $<$<PLATFORM_ID:Windows>:NOMINMAX>
    PUBLIC
        __QT__
)

target_include_directories(
    mbgl-core
    PRIVATE ${PROJECT_SOURCE_DIR}/platform/default/include
)

include(${PROJECT_SOURCE_DIR}/vendor/nunicode.cmake)

set_property(TARGET mbgl-core PROPERTY AUTOMOC ON)
if (Qt6_FOUND AND COMMAND qt_enable_autogen_tool)
    qt_enable_autogen_tool(mbgl-core "moc" ON)
endif()

target_link_libraries(
    mbgl-core
    PUBLIC
        $<BUILD_INTERFACE:mbgl-vendor-parsedate>
        $<BUILD_INTERFACE:mbgl-vendor-nunicode>
        $<BUILD_INTERFACE:mbgl-vendor-csscolorparser>
        $<BUILD_INTERFACE:mlt-cpp>
        $<BUILD_INTERFACE:fastpfor-lib>
        $<$<PLATFORM_ID:iOS>:$<BUILD_INTERFACE:mbgl-vendor-filesystem>>
        $<$<NOT:$<OR:$<PLATFORM_ID:Windows>,$<PLATFORM_ID:Emscripten>>>:z>
        $<IF:$<BOOL:${MLN_QT_WITH_INTERNAL_SQLITE}>,$<BUILD_INTERFACE:mbgl-vendor-sqlite>,Qt${QT_VERSION_MAJOR}::Sql>
    PRIVATE
        $<$<PLATFORM_ID:Linux>:${CMAKE_THREAD_LIBS_INIT}>
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Network
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (MLN_QT_WITH_INTERNAL_ICU)
        target_link_libraries(mbgl-core PUBLIC $<BUILD_INTERFACE:mbgl-vendor-icu>)
    else()
        target_link_libraries(mbgl-core PUBLIC ${ICU_UC_LIBRARIES} ${ICU_I18N_LIBRARIES})
        target_include_directories(mbgl-core PRIVATE ${ICU_UC_INCLUDE_DIRS} ${ICU_I18N_INCLUDE_DIRS})
    endif()
endif()

# Object library list
get_directory_property(MLN_QT_HAS_PARENT PARENT_DIRECTORY)
if(MLN_QT_HAS_PARENT)
    set(MLN_QT_VENDOR_LIBRARIES
        mbgl-vendor-parsedate
        mbgl-vendor-nunicode
        mbgl-vendor-csscolorparser
        mlt-cpp
        fastpfor-lib
        $<$<PLATFORM_ID:iOS>:mbgl-vendor-filesystem>
        $<$<BOOL:${MLN_QT_WITH_INTERNAL_SQLITE}>:mbgl-vendor-sqlite>
        $<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${MLN_QT_WITH_INTERNAL_ICU}>>:mbgl-vendor-icu>
        PARENT_SCOPE
    )
endif()

if(NOT MLN_QT_LIBRARY_ONLY)
    # test runner
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
        PRIVATE
            WORK_DIRECTORY=${PROJECT_SOURCE_DIR}
            $<$<PLATFORM_ID:Windows>:MBGL_BUILDING_LIB>
    )

    target_link_libraries(
        mbgl-test-runner
        PRIVATE
            Qt${QT_VERSION_MAJOR}::Gui
            mbgl-compiler-options
    )

    if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
        target_link_libraries(
            mbgl-test-runner
            PRIVATE
                -Wl,-force_load mbgl-test
        )
    else()
        target_link_libraries(
            mbgl-test-runner
            PRIVATE
                $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-test>
        )
    endif()

    add_test(NAME mbgl-test-runner COMMAND mbgl-test-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
