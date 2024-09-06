if(MLN_WITH_EGL)
    set(_RENDERER EGL)
elseif(MLN_WITH_OSMESA)
    set(_RENDERER OSMesa)
elseif(MLN_WITH_VULKAN)
    set(_RENDERER Vulkan)
else()
    set(_RENDERER OpenGL)
endif()

execute_process(COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_LIST_DIR}/Get-VendorPackages.ps1 -Triplet ${VCPKG_TARGET_TRIPLET} -Renderer ${_RENDERER})
unset(_RENDERER)

add_compile_definitions(NOMINMAX GHC_WIN_DISABLE_WSTRING_STORAGE_TYPE)

target_compile_options(
    mbgl-compiler-options
    INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/MP>
)

find_package(CURL REQUIRED)
find_package(dlfcn-win32 REQUIRED)
find_package(ICU OPTIONAL_COMPONENTS i18n uc)
find_package(JPEG REQUIRED)
find_package(libuv REQUIRED)
find_package(PNG REQUIRED)
find_package(WebP REQUIRED)
find_path(DLFCN_INCLUDE_DIRS dlfcn.h)
find_path(LIBUV_INCLUDE_DIRS uv.h)

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
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/timer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/utf.cpp
        ${PROJECT_SOURCE_DIR}/platform/windows/src/thread.cpp
        ${PROJECT_SOURCE_DIR}/platform/windows/src/thread_local.cpp
)

target_compile_definitions(
    mbgl-core
    PRIVATE
        CURL_STATICLIB
        USE_STD_FILESYSTEM
)

if(MLN_WITH_OPENGL)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gl/headless_backend.cpp
    )
endif()

if(MLN_WITH_EGL)
    find_package(unofficial-angle CONFIG REQUIRED)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/windows/src/headless_backend_egl.cpp
            ${PROJECT_SOURCE_DIR}/platform/windows/src/gl_functions.cpp
    )
    target_compile_definitions(
        mbgl-core
        PRIVATE
            KHRONOS_STATIC
    )
    target_link_libraries(
        mbgl-core
        PRIVATE
            unofficial::angle::libEGL
            unofficial::angle::libGLESv2
    )
elseif(MLN_WITH_OSMESA)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/vendor/mesa3d)

    find_package(OSMesa REQUIRED)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/windows/src/headless_backend_osmesa.cpp
            ${PROJECT_SOURCE_DIR}/platform/windows/src/gl_functions.cpp
    )
    
    set_property(
        SOURCE ${PROJECT_SOURCE_DIR}/platform/windows/src/headless_backend_osmesa.cpp
        PROPERTY INCLUDE_DIRECTORIES ${OSMesa_INCLUDE_DIRS}
    )

    target_link_libraries(
        mbgl-core
        PRIVATE
            OSMesa::osmesa
            OSMesa::libGLESv2
    )
elseif(MLN_WITH_VULKAN)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/vulkan/headless_backend.cpp
    )
else()
    find_package(OpenGL REQUIRED)
    target_sources(
        mbgl-core
        PRIVATE
            ${PROJECT_SOURCE_DIR}/platform/windows/src/headless_backend_wgl.cpp
    )
    target_compile_definitions(
        mbgl-core
        PRIVATE
            KHRONOS_STATIC
    )
    target_link_libraries(
        mbgl-core
        PRIVATE
            OpenGL::GL
    )
endif()

# FIXME: Should not be needed, but now needed by node because of the headless frontend.
target_include_directories(
    mbgl-core
    PUBLIC ${PROJECT_SOURCE_DIR}/platform/default/include
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/windows/include
        ${CURL_INCLUDE_DIRS}
		${DLFCN_INCLUDE_DIRS}
        ${JPEG_INCLUDE_DIRS}
        ${LIBUV_INCLUDE_DIRS}
        ${WEBP_INCLUDE_DIRS}
)

include(${PROJECT_SOURCE_DIR}/vendor/nunicode.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)

if(NOT ${ICU_FOUND} OR "${ICU_VERSION}" VERSION_LESS 62.0)
    message(STATUS "ICU not found or too old, using builtin.")

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
        ${WEBP_LIBRARIES}
		dlfcn-win32::dl
        $<$<NOT:$<BOOL:${MLN_USE_BUILTIN_ICU}>>:ICU::data>
        $<$<NOT:$<BOOL:${MLN_USE_BUILTIN_ICU}>>:ICU::i18n>
        $<$<NOT:$<BOOL:${MLN_USE_BUILTIN_ICU}>>:ICU::uc>
        $<$<BOOL:${MLN_USE_BUILTIN_ICU}>:mbgl-vendor-icu>
        PNG::PNG
        mbgl-vendor-nunicode
        mbgl-vendor-sqlite
)

add_subdirectory(${PROJECT_SOURCE_DIR}/bin)
add_subdirectory(${PROJECT_SOURCE_DIR}/expression-test)
add_subdirectory(${PROJECT_SOURCE_DIR}/platform/glfw)
if(MLN_WITH_NODE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/platform/node)
endif()

add_executable(
    mbgl-test-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/test/main.cpp
)

target_compile_definitions(
    mbgl-test-runner
    PRIVATE MBGL_BUILDING_LIB WORK_DIRECTORY=${PROJECT_SOURCE_DIR}
)

target_include_directories(
    mbgl-test-runner
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/windows/include
)

target_link_libraries(
    mbgl-test-runner
    PRIVATE
        mbgl-compiler-options
        -Wl,--whole-archive
        mbgl-test
        -Wl,--no-whole-archive
)

add_executable(
    mbgl-benchmark-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/benchmark/main.cpp
)

target_link_libraries(
    mbgl-benchmark-runner
    PRIVATE
        mbgl-compiler-options
        mbgl-benchmark
        -WHOLEARCHIVE:mbgl-benchmark
        $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
        shlwapi
)

add_executable(
    mbgl-render-test-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/render-test/main.cpp
)

target_compile_definitions(
    mbgl-render-test-runner
    PRIVATE MBGL_BUILDING_LIB
)

target_link_libraries(
    mbgl-render-test-runner
    PRIVATE
        mbgl-compiler-options
        mbgl-render-test
        $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
)

# Disable benchmarks in CI as they run in VM environment
if(NOT DEFINED ENV{CI})
    add_test(NAME mbgl-benchmark-runner COMMAND mbgl-benchmark-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
add_test(NAME mbgl-test-runner COMMAND mbgl-test-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

install(TARGETS mbgl-render-test-runner RUNTIME DESTINATION bin)
