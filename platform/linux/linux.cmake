option(MLN_WITH_X11 "Build with X11 Support" ON)
option(MLN_WITH_WAYLAND "Build with Wayland Support" OFF)

find_package(CURL REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(PkgConfig REQUIRED)
if (MLN_WITH_X11)
    find_package(X11 REQUIRED)
endif ()
find_package(Threads REQUIRED)

pkg_search_module(WEBP libwebp REQUIRED)
pkg_search_module(LIBUV libuv REQUIRED)
pkg_search_module(ICUUC icu-uc)
pkg_search_module(ICUI18N icu-i18n)
find_program(ARMERGE NAMES armerge)

if(MLN_WITH_WAYLAND AND NOT MLN_WITH_VULKAN)
    # See https://github.com/maplibre/maplibre-native/pull/2022

    # MLN_WITH_EGL needs to be set for Wayland, otherwise this CMakeLists will
    # call find_package(OpenGL REQUIRED GLX), which is for X11.
    set(MLN_WITH_EGL TRUE)

    # OPENGL_USE_GLES2 or OPENGL_USE_GLES3 need to be set, otherwise
    # FindOpenGL.cmake will include the GLVND library, which is for X11.
    set(OPENGL_USE_GLES3 TRUE)
endif()

target_sources(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_backend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_frontend.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/i18n/collator.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/i18n/number_format.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/layermanager/layer_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/platform/time.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/asset_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/database_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/file_source_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/file_source_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/http_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/local_file_request.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/local_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/mbtiles_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/main_resource_loader.cpp
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
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/image.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/jpeg_reader.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/webp_reader.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/logging_stderr.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/monotonic_timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/png_reader.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/png_writer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/run_loop.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/string_stdlib.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/thread.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/thread_local.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/utf.cpp
)

if(MLN_WITH_OPENGL)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gl/headless_backend.cpp
            ${PROJECT_SOURCE_DIR}/platform/linux/src/gl_functions.cpp
    )
endif()

if(MLN_WITH_EGL)
    find_package(OpenGL REQUIRED EGL)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/linux/src/headless_backend_egl.cpp
    )
    target_link_libraries(
        mbgl-core
        PRIVATE
            OpenGL::EGL
    )
    if (MLN_WITH_WAYLAND)
        target_compile_definitions(mbgl-core PUBLIC
                EGL_NO_X11
                MESA_EGL_NO_X11_HEADERS
                WL_EGL_PLATFORM
        )
    endif()
elseif(MLN_WITH_VULKAN)
    target_include_directories(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/vendor/Vulkan-Headers/include
    )

    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/vulkan/headless_backend.cpp
    )
else()
    find_package(OpenGL REQUIRED GLX)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/linux/src/headless_backend_glx.cpp
    )
    target_link_libraries(
        mbgl-core
        PRIVATE
            OpenGL::GLX
    )
endif()

if (DEFINED ENV{CI})
    message("Building for CI")
    target_compile_definitions(
        mbgl-core
        PRIVATE
            CI_BUILD=1
    )
endif()

# FIXME: Should not be needed, but now needed by node because of the headless frontend.
target_include_directories(
    mbgl-core
    PUBLIC ${PROJECT_SOURCE_DIR}/platform/default/include
    PRIVATE
        ${CURL_INCLUDE_DIRS}
        ${JPEG_INCLUDE_DIRS}
        ${LIBUV_INCLUDE_DIRS}
        ${X11_INCLUDE_DIRS}
        ${WEBP_INCLUDE_DIRS}
)

include(${PROJECT_SOURCE_DIR}/vendor/nunicode.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)

if(NOT ${ICUUC_FOUND} OR "${ICUUC_VERSION}" VERSION_LESS 62.0 OR MLN_USE_BUILTIN_ICU)
    message(STATUS "ICU not found, too old or MLN_USE_BUILTIN_ICU requestd, using builtin.")

    set(MLN_USE_BUILTIN_ICU TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)

    set_source_files_properties(
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/i18n/number_format.cpp
        PROPERTIES
        COMPILE_DEFINITIONS
        MBGL_USE_BUILTIN_ICU
    )
endif()

target_link_libraries(
    mbgl-core
    PRIVATE
        ${CURL_LIBRARIES}
        ${JPEG_LIBRARIES}
        ${LIBUV_LIBRARIES}
        ${X11_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${WEBP_LIBRARIES}
        $<$<NOT:$<BOOL:${MLN_USE_BUILTIN_ICU}>>:${ICUUC_LIBRARIES}>
        $<$<NOT:$<BOOL:${MLN_USE_BUILTIN_ICU}>>:${ICUI18N_LIBRARIES}>
        $<$<BOOL:${MLN_USE_BUILTIN_ICU}>:mbgl-vendor-icu>
        PNG::PNG
        mbgl-vendor-nunicode
        mbgl-vendor-sqlite
)

if(MLN_CREATE_AMALGAMATION)
    if ("${ARMERGE}" STREQUAL "MLN_CREATE_AMALGAMATION")
        message(FATAL_ERROR "armerge required when MLN_CREATE_AMALGAMATION=ON")
    endif()
    message(STATUS "Found armerge: ${ARMERGE}")
    include(${PROJECT_SOURCE_DIR}/cmake/find_static_library.cmake)
    set(STATIC_LIBS "")

    find_static_library(STATIC_LIBS NAMES png)
    find_static_library(STATIC_LIBS NAMES z)
    find_static_library(STATIC_LIBS NAMES jpeg)
    find_static_library(STATIC_LIBS NAMES webp)
    find_static_library(STATIC_LIBS NAMES uv uv_a)
    find_static_library(STATIC_LIBS NAMES ssl)
    find_static_library(STATIC_LIBS NAMES crypto)
    find_static_library(STATIC_LIBS NAMES bz2 bzip2)

    if(MLN_WITH_VULKAN)
        find_static_library(STATIC_LIBS NAMES glslang)
        find_static_library(STATIC_LIBS NAMES glslang-default-resource-limits)
        find_static_library(STATIC_LIBS NAMES SPIRV)
        find_static_library(STATIC_LIBS NAMES SPIRV-Tools)
        find_static_library(STATIC_LIBS NAMES SPIRV-Tools-opt)
        find_static_library(STATIC_LIBS NAMES MachineIndependent)
        find_static_library(STATIC_LIBS NAMES GenericCodeGen)
    endif()

    add_custom_command(
        TARGET mbgl-core
        POST_BUILD
        COMMAND armerge --keep-symbols 'mbgl.*' --output libmbgl-core-amalgam.a
            $<TARGET_FILE:mbgl-core>
            $<TARGET_FILE:mbgl-freetype>
            $<TARGET_FILE:mbgl-vendor-csscolorparser>
            $<TARGET_FILE:mbgl-harfbuzz>
            $<TARGET_FILE:mbgl-vendor-nunicode>
            $<TARGET_FILE:mbgl-vendor-sqlite>
            $<TARGET_FILE:mbgl-vendor-parsedate>
            ${ICUUC_LIBRARY_DIRS}/libicuuc.a
            ${ICUUC_LIBRARY_DIRS}/libicudata.a
            ${ICUI18N_LIBRARY_DIRS}/libicui18n.a
            ${STATIC_LIBS}
    )

endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/bin)
add_subdirectory(${PROJECT_SOURCE_DIR}/expression-test)
if(MLN_WITH_GLFW)
	add_subdirectory(${PROJECT_SOURCE_DIR}/platform/glfw)
endif()
if(MLN_WITH_NODE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/platform/node)
endif()

add_executable(
    mbgl-test-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/test/main.cpp
)

target_compile_definitions(
    mbgl-test-runner
    PRIVATE
        WORK_DIRECTORY=${PROJECT_SOURCE_DIR}
)

if (DEFINED ENV{CI})
    message("Building for CI")
    target_compile_definitions(
        mbgl-test-runner
        PRIVATE
            CI_BUILD=1
    )
endif()

target_link_libraries(
    mbgl-test-runner
    PRIVATE
        mbgl-compiler-options
        $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-test>
)

add_executable(
    mbgl-benchmark-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/benchmark/main.cpp
)

target_link_libraries(
    mbgl-benchmark-runner
    PRIVATE
        mbgl-compiler-options
        $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-benchmark>
)

add_executable(
    mbgl-render-test-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/render-test/main.cpp
)

target_link_libraries(
    mbgl-render-test-runner
    PRIVATE mbgl-compiler-options mbgl-render-test
)

# Disable benchmarks in CI as they run in VM environment
if(NOT DEFINED ENV{CI})
    add_test(NAME mbgl-benchmark-runner COMMAND mbgl-benchmark-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
add_test(NAME mbgl-test-runner COMMAND mbgl-test-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

install(TARGETS mbgl-render-test-runner RUNTIME DESTINATION bin)
