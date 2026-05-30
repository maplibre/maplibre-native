if(NOT CMAKE_SYSTEM_NAME STREQUAL OHOS)
    message(FATAL_ERROR "OpenHarmony platform support requires the OHOS CMake toolchain")
endif()

option(MLN_OHOS_WITH_HMS_RCP "Use HarmonyOS HMS RemoteCommunicationKit for HTTP requests" OFF)
option(MLN_OHOS_BUILD_LINK_SMOKE "Build an OHOS shared-library link smoke target" OFF)
option(MLN_OHOS_BUILD_NATIVE_MODULE "Build the experimental OHOS NAPI/XComponent native module" OFF)

include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/nunicode.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)

if(MLN_OHOS_WITH_HMS_RCP)
    find_library(MLN_OHOS_RCP_C_LIBRARY NAMES rcp_c REQUIRED)
endif()

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
        ${PROJECT_SOURCE_DIR}/platform/ohos/src/$<IF:$<BOOL:${MLN_OHOS_WITH_HMS_RCP}>,http_file_source_hms_rcp.cpp,http_file_source.cpp>
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
        $<$<NOT:$<BOOL:${MLN_OHOS_WITH_HMS_RCP}>>:net_http>
        pixelmap
        hilog_ndk.z
        $<$<BOOL:${MLN_OHOS_WITH_HMS_RCP}>:${MLN_OHOS_RCP_C_LIBRARY}>
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

function(mln_ohos_link_core_whole_archive target)
    add_dependencies(${target} mbgl-core)

    target_link_libraries(
        ${target}
        PRIVATE
            -Wl,--whole-archive
            $<TARGET_FILE:mbgl-core>
            -Wl,--no-whole-archive
            mbgl-core
    )

    target_link_options(
        ${target}
        PRIVATE
            LINKER:--no-undefined
    )
endfunction()

if(MLN_OHOS_BUILD_LINK_SMOKE)
    add_library(
        mbgl-ohos-link-smoke
        SHARED
            ${PROJECT_SOURCE_DIR}/platform/ohos/src/link_smoke.cpp
    )

    mln_ohos_link_core_whole_archive(mbgl-ohos-link-smoke)
endif()

if(MLN_OHOS_BUILD_NATIVE_MODULE)
    add_library(
        maplibre-native-ohos
        SHARED
            ${PROJECT_SOURCE_DIR}/platform/ohos/src/egl_window_backend.cpp
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

    mbgl_enable_ohos_libcxx_experimental(maplibre-native-ohos)

    target_link_libraries(
        maplibre-native-ohos
        PRIVATE
            mbgl-compiler-options
            ace_napi.z
            ace_ndk.z
            EGL
            GLESv3
            hilog_ndk.z
            native_window
    )

    mln_ohos_link_core_whole_archive(maplibre-native-ohos)
endif()
