if(NOT CMAKE_SYSTEM_NAME STREQUAL OHOS)
    message(FATAL_ERROR "OpenHarmony platform support requires the OpenHarmony CMake toolchain")
endif()

include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/dependency/nunicode.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/dependency/sqlite.cmake)

find_library(MLN_OHOS_NET_HTTP_LIBRARY NAMES net_http REQUIRED)

set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/platform/default/src/mln/i18n/number_format.cpp
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
        ${MLN_DEFAULT_FILE_SOURCE_SOURCES}
        ${MLN_DEFAULT_HEADLESS_SOURCES}
        ${MLN_DEFAULT_LIBUV_RUN_LOOP_SOURCES}
        ${MLN_DEFAULT_LOCALE_SOURCES}
        ${MLN_DEFAULT_UNICODE_SOURCES}
        ${MLN_DEFAULT_UTIL_SOURCES}
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/layermanager/layer_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/map/map_snapshotter.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/sqlite3.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/text/local_glyph_rasterizer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/png_writer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/thread.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/thread_local.cpp
        ${PROJECT_SOURCE_DIR}/platform/ohos/src/http_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/ohos/src/image.cpp
        ${PROJECT_SOURCE_DIR}/platform/ohos/src/logging_hilog.cpp
)

if(MLN_WITH_OPENGL)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/linux/src/gl_functions.cpp
            ${PROJECT_SOURCE_DIR}/platform/linux/src/headless_backend_egl.cpp
    )
endif()

if(MLN_WITH_VULKAN)
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
    mln_pkg_config_libs(-lvulkan)
endif()

target_include_directories(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/default/include
)

# TEMPORARY WORKAROUND: the OpenHarmony SDK's clang rejects the missing `typename`
# at vendor/maplibre-tile-spec/cpp/include/mlt/common.hpp:25. Inject a patched copy
# of that header ahead of the vendored include directory for every target that
# compiles MLT code. The real fix is upstreaming `typename` to
# maplibre/maplibre-tile-spec and bumping the vendored submodule; delete this block
# and platform/ohos/compat/mlt/common.hpp once that lands.
target_include_directories(
    mbgl-core
    BEFORE
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/ohos/compat
)

foreach(MLN_OHOS_MLT_TARGET mlt-cpp mlt-cpp-encoder)
    if(TARGET ${MLN_OHOS_MLT_TARGET})
        target_include_directories(
            ${MLN_OHOS_MLT_TARGET}
            BEFORE
            PRIVATE
                ${PROJECT_SOURCE_DIR}/platform/ohos/compat
        )
    endif()
endforeach()

target_link_libraries(
    mbgl-core
    PRIVATE
        image_source
        mbgl-vendor-icu
        mbgl-nunicode
        mbgl-sqlite
        pixelmap
        hilog_ndk.z
        ${MLN_OHOS_NET_HTTP_LIBRARY}
        uv
        z
)
mln_pkg_config_libs(-limage_source -lpixelmap -lhilog_ndk.z ${MLN_OHOS_NET_HTTP_LIBRARY} -luv -lz)

if(MLN_WITH_OPENGL)
    target_link_libraries(
        mbgl-core
        PRIVATE
            EGL
            GLESv3
    )
    mln_pkg_config_libs(-lEGL -lGLESv3)
endif()

function(mln_ohos_link_core_whole_archive target)
    # Stock CMake has no OHOS definition for this feature. Define it in the
    # consumer's directory scope, where the link expression is evaluated.
    set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "-Wl,--whole-archive <LIBRARY> -Wl,--no-whole-archive" PARENT_SCOPE)
    set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED True PARENT_SCOPE)

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
