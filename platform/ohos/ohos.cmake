if(NOT CMAKE_SYSTEM_NAME STREQUAL OHOS)
    message(FATAL_ERROR "HarmonyOS platform support requires the OHOS/HarmonyOS CMake toolchain")
endif()

option(MLN_OHOS_BUILD_NATIVE_MODULE "Build the experimental OHOS NAPI/XComponent native module" OFF)

include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/nunicode.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)

find_library(MLN_OHOS_RCP_C_LIBRARY NAMES rcp_c REQUIRED)

set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/i18n/number_format.cpp
    PROPERTIES
        COMPILE_DEFINITIONS MBGL_USE_BUILTIN_ICU
)

target_compile_definitions(
    mbgl-core
    PRIVATE
        OHOS_PLATFORM
)

target_sources(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_backend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_frontend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/i18n/collator.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/i18n/number_format.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/layermanager/layer_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/map/map_snapshotter.cpp
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
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/sqlite3.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/text/bidi.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/text/local_glyph_rasterizer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/async_task.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/compression.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/filesystem.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/monotonic_timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/png_writer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/run_loop.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/string_stdlib.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/thread.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/thread_local.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/utf.cpp
        ${PROJECT_SOURCE_DIR}/platform/ohos/src/http_file_source_hms_rcp.cpp
        ${PROJECT_SOURCE_DIR}/platform/ohos/src/image.cpp
        ${PROJECT_SOURCE_DIR}/platform/ohos/src/logging_hilog.cpp
)

if(MLN_WITH_OPENGL)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gl/headless_backend.cpp
            ${PROJECT_SOURCE_DIR}/platform/linux/src/gl_functions.cpp
            ${PROJECT_SOURCE_DIR}/platform/linux/src/headless_backend_egl.cpp
    )
endif()

if(MLN_WITH_VULKAN)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/vulkan/headless_backend.cpp
    )

    target_compile_definitions(
        mbgl-core
        PUBLIC
            VK_USE_PLATFORM_OHOS=1
    )

    target_link_libraries(
        mbgl-core
        PRIVATE
            vulkan
    )
endif()

target_include_directories(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/default/include
)

target_link_libraries(
    mbgl-core
    PRIVATE
        image_source
        mbgl-vendor-icu
        mbgl-vendor-nunicode
        mbgl-vendor-sqlite
        pixelmap
        hilog_ndk.z
        ${MLN_OHOS_RCP_C_LIBRARY}
        uv
        z
)

if(MLN_WITH_OPENGL)
    target_link_libraries(
        mbgl-core
        PRIVATE
            EGL
            GLESv3
    )
endif()

# The OHOS toolchain is not listed as a built-in CMake platform for this
# feature, so teach CMake how to whole-archive static targets for the native
# module link.
set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "-Wl,--whole-archive <LIBRARY> -Wl,--no-whole-archive")
set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED True)

function(mln_ohos_link_core_whole_archive target)
    target_link_libraries(
        ${target}
        PRIVATE
            $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-core>
    )

    target_link_options(
        ${target}
        PRIVATE
            LINKER:--no-undefined
    )
endfunction()

if(MLN_OHOS_BUILD_NATIVE_MODULE)
    if(MLN_WITH_VULKAN)
        set(MLN_OHOS_WINDOW_BACKEND_SOURCE ${PROJECT_SOURCE_DIR}/platform/ohos/src/vulkan_window_backend.cpp)
    else()
        set(MLN_OHOS_WINDOW_BACKEND_SOURCE ${PROJECT_SOURCE_DIR}/platform/ohos/src/egl_window_backend.cpp)
    endif()

    add_library(
        maplibre-native-ohos
        SHARED
            ${MLN_OHOS_WINDOW_BACKEND_SOURCE}
            ${PROJECT_SOURCE_DIR}/platform/ohos/src/gesture_handler.cpp
            ${PROJECT_SOURCE_DIR}/platform/ohos/src/map_view.cpp
            ${PROJECT_SOURCE_DIR}/platform/ohos/src/native_module.cpp
            ${PROJECT_SOURCE_DIR}/platform/ohos/src/native_values.cpp
            ${PROJECT_SOURCE_DIR}/platform/ohos/src/renderer_frontend.cpp
    )

    set_target_properties(
        maplibre-native-ohos
        PROPERTIES
            OUTPUT_NAME maplibre_native_ohos
    )

    target_compile_definitions(
        maplibre-native-ohos
        PRIVATE
            OHOS_PLATFORM
    )

    target_include_directories(
        maplibre-native-ohos
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src
    )

    mbgl_enable_ohos_libcxx_experimental(maplibre-native-ohos)

    target_link_libraries(
        maplibre-native-ohos
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

    mln_ohos_link_core_whole_archive(maplibre-native-ohos)
endif()
