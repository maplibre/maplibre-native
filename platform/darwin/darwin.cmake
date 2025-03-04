enable_language(OBJC OBJCXX Swift)

target_link_libraries(
    mbgl-core
    PRIVATE
        "-framework CoreGraphics"
        "-framework CoreLocation"
        "-framework CoreImage"
        "-framework SystemConfiguration"
        mbgl-vendor-icu
        sqlite3
        z
)

target_sources(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/async_task.cpp
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/collator.mm
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/http_file_source.mm
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/image.mm
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/local_glyph_rasterizer.mm
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/logging_nslog.mm
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/native_apple_interface.m
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/nsthread.mm
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/number_format.mm
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/run_loop.cpp
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/string_nsstring.mm
        ${PROJECT_SOURCE_DIR}/platform/darwin/core/timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_backend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_frontend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/layermanager/layer_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/map/map_snapshotter.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/platform/time.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/asset_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/mbtiles_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/database_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/file_source_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/file_source_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/local_file_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/local_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/main_resource_loader.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/offline.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/offline_database.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/offline_download.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/online_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/$<IF:$<BOOL:${MLN_WITH_PMTILES}>,pmtiles_file_source.cpp,pmtiles_file_source_stub.cpp>
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/sqlite3.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/text/bidi.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/compression.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/filesystem.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/monotonic_timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/png_writer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/thread_local.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/utf.cpp
)

target_include_directories(
    mbgl-core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/platform/darwin/include
        ${PROJECT_SOURCE_DIR}/platform/default/include
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/darwin/src ${PROJECT_SOURCE_DIR}/platform/macos/src
)

if(MLN_WITH_METAL)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/mtl/headless_backend.cpp
    )
endif()

include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)

set(CMAKE_OBJC_FLAGS "-fobjc-arc")
set(CMAKE_OBJCXX_FLAGS "-fobjc-arc")

set(MLN_GENERATED_DARWIN_CODE_DIR
    ${CMAKE_BINARY_DIR}/generated-darwin-code/src
)

set(MLN_GENERATED_DARWIN_STYLE_SOURCE
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNLight.mm"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNBackgroundStyleLayer.mm"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNCircleStyleLayer.mm"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNFillExtrusionStyleLayer.mm"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNFillStyleLayer.mm"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNHeatmapStyleLayer.mm"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNHillshadeStyleLayer.mm"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNLineStyleLayer.mm"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNRasterStyleLayer.mm"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNSymbolStyleLayer.mm"
)

set(MLN_GENERATED_DARWIN_STYLE_PUBLIC_HEADERS
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNBackgroundStyleLayer.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNFillExtrusionStyleLayer.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNHeatmapStyleLayer.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNLight.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNLineStyleLayer.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNSymbolStyleLayer.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNCircleStyleLayer.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNFillStyleLayer.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNHillshadeStyleLayer.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNRasterStyleLayer.h"
)

set(MLN_GENERATED_DARWIN_STYLE_HEADERS
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNRasterStyleLayer_Private.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNBackgroundStyleLayer_Private.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNFillExtrusionStyleLayer_Private.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNHeatmapStyleLayer_Private.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNLineStyleLayer_Private.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNSymbolStyleLayer_Private.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNCircleStyleLayer_Private.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNFillStyleLayer_Private.h"
    "${MLN_GENERATED_DARWIN_CODE_DIR}/MLNHillshadeStyleLayer_Private.h"
    ${MLN_GENERATED_DARWIN_STYLE_PUBLIC_HEADERS}
)

find_program(BAZEL bazel REQUIRED)

add_custom_command(
    OUTPUT ${MLN_GENERATED_DARWIN_STYLE_SOURCE} ${MLN_GENERATED_DARWIN_STYLE_HEADERS}
    COMMAND ${CMAKE_COMMAND} -E rm -Rf
        "${PROJECT_SOURCE_DIR}/bazel-bin/platform/darwin/src"
    COMMAND ${BAZEL} build //platform/darwin:generated_code
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/bazel-bin/platform/darwin/src"
        ${MLN_GENERATED_DARWIN_CODE_DIR}
    COMMENT "Generating Darwin style source and header files"
    VERBATIM
)

add_custom_target(mbgl-darwin-style-code
    DEPENDS ${MLN_GENERATED_DARWIN_STYLE_SOURCE} ${MLN_GENERATED_DARWIN_STYLE_HEADERS}
)

add_library(
    custom-layer-examples
    EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_LIST_DIR}/app/ExampleCustomDrawableStyleLayer.mm"
    "${CMAKE_CURRENT_LIST_DIR}/app/CustomStyleLayerExample.m"
)

target_link_libraries(
    custom-layer-examples
    PUBLIC ios-sdk-static
    PRIVATE mbgl-compiler-options mbgl-core
)

if(MLN_WITH_METAL)
    target_compile_definitions(
        custom-layer-examples
        PRIVATE MLN_RENDER_BACKEND_METAL=1
    )
endif()

target_include_directories(
    custom-layer-examples
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/app"
    PRIVATE
        "${PROJECT_SOURCE_DIR}/src" # FIXME: should not use private headers
)
