if(MSVC)
    if(MLN_WITH_EGL)
        set(_RENDERER EGL)
    elseif(MLN_WITH_VULKAN)
        set(_RENDERER Vulkan)
    else()
        set(_RENDERER OpenGL)
    endif()

    if(NOT MLN_USE_BUILTIN_ICU)
        set(WITH_ICU -With-ICU)
    endif()

    execute_process(COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_LIST_DIR}/Get-VendorPackages.ps1 -Triplet ${VCPKG_TARGET_TRIPLET} -Renderer ${_RENDERER} ${WITH_ICU})
    unset(_RENDERER)

    add_compile_definitions(NOMINMAX GHC_WIN_DISABLE_WSTRING_STORAGE_TYPE)

    target_compile_options(
        mbgl-compiler-options
        INTERFACE
            /MP
    )

    find_package(CURL REQUIRED)
    find_package(ICU OPTIONAL_COMPONENTS i18n uc)
    find_package(JPEG REQUIRED)
    find_package(PNG REQUIRED)
    find_package(WebP REQUIRED)
elseif(DEFINED ENV{MSYSTEM})
    set(MSYS 1)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-static")

    add_compile_definitions(WIN32 GHC_WIN_DISABLE_WSTRING_STORAGE_TYPE)

    find_package(ICU OPTIONAL_COMPONENTS i18n uc data)
    find_package(JPEG REQUIRED)
    find_package(PNG REQUIRED)
    find_package(PkgConfig REQUIRED)

    pkg_search_module(WEBP libwebp REQUIRED)
    pkg_search_module(CURL libcurl REQUIRED)
else()
    message(FATAL_ERROR "Unsupported build system: " ${CMAKE_SYSTEM_NAME})
endif()

target_sources(
    mbgl-core
    PRIVATE
        ${MLN_DEFAULT_FILE_SOURCE_SOURCES}
        ${MLN_DEFAULT_HEADLESS_SOURCES}
        ${MLN_DEFAULT_IMAGE_DECODER_SOURCES}
        ${MLN_DEFAULT_LOCALE_SOURCES}
        ${MLN_DEFAULT_UNICODE_SOURCES}
        ${MLN_DEFAULT_UTIL_SOURCES}
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/layermanager/layer_manager.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/http_file_source.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/storage/sqlite3.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/text/local_glyph_rasterizer.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/logging_stderr.cpp
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/util/png_writer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/thread.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/thread_local.cpp
)

target_compile_definitions(
    mbgl-core
    PRIVATE
        CURL_STATICLIB
        USE_STD_FILESYSTEM
)

if(MLN_WITH_EGL)
    if(MSVC)
        find_package(unofficial-angle CONFIG REQUIRED)

        target_link_libraries(
            mbgl-core
            PRIVATE
                unofficial::angle::libEGL
                unofficial::angle::libGLESv2
        )
    elseif(MSYS)
        pkg_search_module(EGL angleproject REQUIRED)

        target_link_libraries(
            mbgl-core
            PRIVATE
                ${EGL_LIBRARIES}
        )
    endif()

    target_sources(
        mbgl-core
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/src/headless_backend_egl.cpp
            ${PROJECT_SOURCE_DIR}/platform/default/src/mln/gl/gl_functions.cpp
    )
    target_compile_definitions(
        mbgl-core
        PRIVATE
            KHRONOS_STATIC
    )
elseif(NOT MLN_WITH_VULKAN)
    find_package(OpenGL REQUIRED)
    target_sources(
        mbgl-core
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/src/headless_backend_wgl.cpp
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
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CURL_INCLUDE_DIRS}
        ${JPEG_INCLUDE_DIRS}
        ${WEBP_INCLUDE_DIRS}
)

include(${PROJECT_SOURCE_DIR}/vendor/nunicode.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)

if(NOT ${ICU_FOUND} OR "${ICU_VERSION}" VERSION_LESS 62.0 OR MLN_USE_BUILTIN_ICU)
    message(STATUS "ICU not found, too old or MLN_USE_BUILTIN_ICU requestd, using builtin.")

    set(MLN_USE_BUILTIN_ICU TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)

    set_source_files_properties(
        ${PROJECT_SOURCE_DIR}/platform/default/src/mln/i18n/number_format.cpp
        PROPERTIES
        COMPILE_DEFINITIONS
        MBGL_USE_BUILTIN_ICU
    )

    target_compile_definitions(
        mbgl-vendor-icu
        PRIVATE
            U_STATIC_IMPLEMENTATION
    )

    target_include_directories(
        mbgl-core
        BEFORE
        PRIVATE
            ${PROJECT_SOURCE_DIR}/vendor/icu/include
    )
elseif(MSYS)
    target_compile_definitions(
        mbgl-core
        PRIVATE
            U_STATIC_IMPLEMENTATION
    )
endif()

if(MSVC)
    target_link_libraries(
        mbgl-core
        PRIVATE
            ${CURL_LIBRARIES}
    )
elseif(MSYS)
    target_link_libraries(
        mbgl-core
        PRIVATE
            ${CURL_STATIC_LIBRARIES}
    )
endif()

target_link_libraries(
    mbgl-core
    PRIVATE
        ${JPEG_LIBRARIES}
        ${WEBP_LIBRARIES}
        $<$<NOT:$<BOOL:${MLN_USE_BUILTIN_ICU}>>:ICU::i18n>
        $<$<NOT:$<BOOL:${MLN_USE_BUILTIN_ICU}>>:ICU::uc>
        $<$<NOT:$<BOOL:${MLN_USE_BUILTIN_ICU}>>:ICU::data>
        $<$<BOOL:${MLN_USE_BUILTIN_ICU}>:mbgl-vendor-icu>
        PNG::PNG
        mbgl-vendor-nunicode
        mbgl-vendor-sqlite
)

# The expression test and the default platform's runners work here, and are
# added at the top level.
set(MLN_DEFAULT_TEST_RUNNERS TRUE)

# The default run loop, with libuv, which the top level adds.
set(MLN_USE_LIBUV TRUE)
