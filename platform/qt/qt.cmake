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

        find_package(ICU COMPONENTS uc REQUIRED)

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

target_sources(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/$<IF:$<PLATFORM_ID:Linux>,default/src/mbgl/text/bidi.cpp,qt/src/mbgl/bidi.cpp>

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
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/$<IF:$<BOOL:${MLN_WITH_PMTILES}>,pmtiles_file_source.cpp,pmtiles_file_source_stub.cpp>
        ${PROJECT_SOURCE_DIR}/platform/$<IF:$<BOOL:${MLN_QT_WITH_INTERNAL_SQLITE}>,default/src/mbgl/storage/sqlite3.cpp,qt/src/mbgl/sqlite3.cpp>
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/compression.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/filesystem.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/monotonic_timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/async_task.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/async_task_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/gl_functions.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/http_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/http_file_source.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/http_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/http_request.hpp
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
        ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/renderer_observer.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/scheduler.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/scheduler.hpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/backend_flag.cpp
)


# Add Qt Metal renderer backend only when Metal backend is enabled
if(MLN_WITH_METAL)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/buffer_resource.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/command_encoder.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/context.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/drawable.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/drawable_builder.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/index_buffer_resource.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/layer_group.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/mtl.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/offscreen_texture.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/render_pass.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/renderer_backend.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/texture2d.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/tile_layer_group.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/uniform_buffer.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/upload_pass.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/vertex_attribute.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/mtl/vertex_buffer_resource.cpp

            # Shader and style Metal
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/background.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/circle.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/clipping_mask.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/collision.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/custom_geometry.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/custom_symbol_icon.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/debug.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/fill.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/fill_extrusion.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/heatmap.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/heatmap_texture.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/hillshade.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/hillshade_prepare.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/line.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/location_indicator.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/raster.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/shader_program.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/symbol.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/shaders/mtl/widevector.cpp
            ${PROJECT_SOURCE_DIR}/src/mbgl/style/layers/mtl/custom_layer_render_parameters.cpp

            ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/metal_renderer_backend.mm
            ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/metal_renderer_backend.hpp
            ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/renderer_backend.hpp
    )
endif()

# Add Qt OpenGL renderer backend only when OpenGL backend is enabled
if(MLN_WITH_OPENGL)
    target_sources(
        mbgl-core
        PRIVATE

        # Generic headless helpers are only relevant for OpenGL builds.
        # Exclude them when building Metal/Vulkan-only to avoid pulling GL symbols.
            ${PROJECT_SOURCE_DIR}/platform/default/include/mbgl/gfx/headless_backend.hpp>
            ${PROJECT_SOURCE_DIR}/platform/default/include/mbgl/gfx/headless_frontend.hpp>
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_backend.cpp>
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_frontend.cpp>
            ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/headless_backend_qt.cpp>

            ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/opengl_renderer_backend.cpp
            ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/opengl_renderer_backend.hpp
            ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/renderer_backend.hpp
    )
endif()

# Add Qt Vulkan renderer backend only when Vulkan backend is enabled
if(MLN_WITH_VULKAN)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/vulkan_renderer_backend.cpp
            ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/vulkan_renderer_backend.hpp
            ${PROJECT_SOURCE_DIR}/platform/qt/src/utils/renderer_backend.hpp
    )
endif()

target_compile_definitions(
    mbgl-core
    PRIVATE QT_IMAGE_DECODERS
    PUBLIC __QT__
)

# On macOS, libc++ defines __cpp_lib_format while still marking 'to_chars' for
# floating-point as unavailable for deployment targets < 13.3, causing the
# Vulkan headers to trip over it.  Force-undefine the macro before compilation
# so that the header falls back to its safe <sstream> path.
target_compile_options(
    mbgl-core
    PRIVATE
        $<$<PLATFORM_ID:Darwin>:-U__cpp_lib_format -D__cpp_lib_format=0>
)

target_include_directories(
    mbgl-core
    PRIVATE ${PROJECT_SOURCE_DIR}/platform/default/include
            ${PROJECT_SOURCE_DIR}/vendor/metal-cpp
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
        target_link_libraries(mbgl-core PUBLIC ICU::uc)
    endif()
endif()

# Object library list
get_directory_property(MLN_QT_HAS_PARENT PARENT_DIRECTORY)
if(MLN_QT_HAS_PARENT)
    set(MLN_QT_VENDOR_LIBRARIES
        mbgl-vendor-parsedate
        mbgl-vendor-nunicode
        mbgl-vendor-csscolorparser
        $<$<BOOL:${MLN_QT_WITH_INTERNAL_SQLITE}>:$<BUILD_INTERFACE:mbgl-vendor-sqlite>>
        $<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${MLN_QT_WITH_INTERNAL_ICU}>>:$<BUILD_INTERFACE:mbgl-vendor-icu>>
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
            PRIVATE -Wl,-force_load mbgl-test
        )
    else()
        target_link_libraries(
            mbgl-test-runner
            PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-test>
        )
    endif()

    add_test(NAME mbgl-test-runner COMMAND mbgl-test-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

# Metal backend sources (macOS only)
if(APPLE AND MLN_WITH_METAL)
    target_link_libraries(
        mbgl-core
        PRIVATE
            "-framework QuartzCore"
            "-framework Metal"
            "-framework CoreFoundation"
    )
    target_compile_definitions(
        mbgl-core
        PRIVATE MLN_RENDER_BACKEND_METAL=1
    )
endif()

# Vulkan backend sources (macOS only)
if(APPLE AND MLN_WITH_VULKAN)
    target_link_libraries(
        mbgl-core
        PRIVATE
            "-framework QuartzCore"
            "-framework CoreFoundation"
    )
    target_compile_definitions(
        mbgl-core
        PRIVATE MLN_RENDER_BACKEND_VULKAN=1
    )
endif()

# ---------------------------------------------------------------------------
# Exclude generic head-less helpers when we build Metal-only (no OpenGL)
# ---------------------------------------------------------------------------
if(NOT MLN_WITH_OPENGL)
    set_source_files_properties(
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_backend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_frontend.cpp
        ${PROJECT_SOURCE_DIR}/platform/qt/src/mbgl/headless_backend_qt.cpp
        PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
