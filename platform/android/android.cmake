if(NOT ANDROID_NDK_TOOLCHAIN_INCLUDED)
    message(FATAL_ERROR "-- Toolchain file not included, see https://developer.android.com/ndk/guides/cmake")
endif()

target_compile_definitions(
    mbgl-core
    PUBLIC
)

include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)

# cmake-format: off
# Apply size optimizations for both Release and RelWithDebInfo configs.
# Android AGP maps the release build variant to RelWithDebInfo (not Release), so we must
# include RelWithDebInfo here to ensure these flags are actually applied.
set(_OPT_CONFIGS "$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>")
target_compile_options(mbgl-vendor-csscolorparser PRIVATE $<${_OPT_CONFIGS}:-Oz> $<${_OPT_CONFIGS}:-Qunused-arguments> $<${_OPT_CONFIGS}:-flto>)
target_compile_options(mbgl-vendor-icu PRIVATE $<${_OPT_CONFIGS}:-Oz> $<${_OPT_CONFIGS}:-Qunused-arguments> $<${_OPT_CONFIGS}:-flto>)
target_compile_options(mbgl-vendor-parsedate PRIVATE $<${_OPT_CONFIGS}:-Oz> $<${_OPT_CONFIGS}:-Qunused-arguments> $<${_OPT_CONFIGS}:-flto>)
target_compile_options(mbgl-vendor-sqlite PRIVATE $<${_OPT_CONFIGS}:-Oz> $<${_OPT_CONFIGS}:-Qunused-arguments> $<${_OPT_CONFIGS}:-flto>)
target_compile_options(mbgl-compiler-options INTERFACE $<${_OPT_CONFIGS}:-Oz> $<${_OPT_CONFIGS}:-Qunused-arguments> $<${_OPT_CONFIGS}:-flto>)
# cmake-format: on

target_link_libraries(
    mbgl-compiler-options
    INTERFACE
        $<${_OPT_CONFIGS}:-O2>
        $<${_OPT_CONFIGS}:-Wl,--icf=all>
        $<${_OPT_CONFIGS}:-flto>
        $<${_OPT_CONFIGS}:-fuse-ld=lld>
)

target_sources(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/android/src/async_task.cpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/attach_env.cpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/attach_env.hpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/bitmap.cpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/bitmap.hpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/bitmap_factory.cpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/bitmap_factory.hpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/image.cpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/jni.cpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/jni.hpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/run_loop.cpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/run_loop_impl.hpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/string_util.cpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/thread.cpp
        ${PROJECT_SOURCE_DIR}/platform/android/src/timer.cpp
        ${MLN_DEFAULT_FILE_SOURCE_SOURCES}
        ${MLN_DEFAULT_HEADLESS_SOURCES}
        ${MLN_DEFAULT_UNICODE_SOURCES}
        ${MLN_DEFAULT_UTIL_SOURCES}
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/map/map_snapshotter.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/sqlite3.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/png_writer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/thread_local.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/layermanager/layer_manager.cpp
)

if(MLN_WITH_OPENGL)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/linux/src/headless_backend_egl.cpp
            ${PROJECT_SOURCE_DIR}/platform/android/src/gl_functions.cpp
    )
endif()

if(MLN_WITH_WEBGPU)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src/mln/webgpu/headless_backend.cpp
    )
endif()

target_include_directories(
    mbgl-core
    PRIVATE ${PROJECT_SOURCE_DIR}/platform/default/include
)

target_link_libraries(
    mbgl-core
    PRIVATE
        EGL
        GLESv3
        MapLibreNative::Base::jni.hpp
        android
        atomic
        jnigraphics
        log
        mbgl-vendor-icu
        mbgl-vendor-sqlite
        z
)

add_library(
    example-custom-layer MODULE
    ${PROJECT_SOURCE_DIR}/platform/android/src/example_custom_layer.cpp
)

target_include_directories(
    example-custom-layer
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    example-custom-layer
    PRIVATE
        GLESv3
        MapLibreNative::Base
        log
        mbgl-compiler-options
)

if(MLN_WITH_VULKAN)
    add_library(
        example-vulkan-custom-layer MODULE
        ${PROJECT_SOURCE_DIR}/platform/android/src/example_vulkan_custom_layer.cpp
    )

    target_include_directories(
        example-vulkan-custom-layer
        PRIVATE ${PROJECT_SOURCE_DIR}/include
    )

    target_link_libraries(
        example-vulkan-custom-layer
        PRIVATE
            MapLibreNative::Base
            log
            mbgl-compiler-options
            mbgl-vendor-vulkan-headers
    )
endif()

# this is needed because Android is not officially supported
# https://discourse.cmake.org/t/error-when-crosscompiling-with-whole-archive-target-link/9394
# https://cmake.org/cmake/help/latest/release/3.24.html#generator-expressions
set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE
"-Wl,--whole-archive <LIBRARY> -Wl,--no-whole-archive"
)
set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED True)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/test)
