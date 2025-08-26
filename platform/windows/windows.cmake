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

    # Handle ARM64 package management
    set(IS_ARM64_BUILD FALSE)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64" OR CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        set(IS_ARM64_BUILD TRUE)
    endif()

    if(NOT IS_ARM64_BUILD)
        # Standard package installation for x86/x64
        execute_process(COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_LIST_DIR}/Get-VendorPackages.ps1 -Triplet ${VCPKG_TARGET_TRIPLET} -Renderer ${_RENDERER} ${WITH_ICU})
    else()
        # ARM64-specific package handling
        message(STATUS "ARM64 build detected - checking for pre-installed packages")
        
        # Try to run the enhanced script that handles ARM64
        execute_process(
            COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_LIST_DIR}/Get-VendorPackages.ps1 -Triplet ${VCPKG_TARGET_TRIPLET} -Renderer ${_RENDERER} ${WITH_ICU}
            RESULT_VARIABLE VCPKG_RESULT
            OUTPUT_VARIABLE VCPKG_OUTPUT
            ERROR_VARIABLE VCPKG_ERROR
        )
        
        if(NOT VCPKG_RESULT EQUAL 0)
            message(WARNING "vcpkg package installation failed for ARM64. Error: ${VCPKG_ERROR}")
            message(STATUS "Assuming packages are pre-installed or will be handled manually")
        endif()
        
        # Set up fallback paths for ARM64 packages if vcpkg fails
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/x64-windows")
            message(STATUS "Using x64 package fallback for ARM64 build")
            set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/x64-windows" ${CMAKE_PREFIX_PATH})
        endif()
    endif()
    
    unset(_RENDERER)

    add_compile_definitions(NOMINMAX GHC_WIN_DISABLE_WSTRING_STORAGE_TYPE)

    target_compile_options(
        mbgl-compiler-options
        INTERFACE
            /MP
    )

    find_package(CURL REQUIRED)
    find_package(dlfcn-win32 REQUIRED)
    find_package(ICU OPTIONAL_COMPONENTS i18n uc)
    find_package(JPEG REQUIRED)
    find_package(libuv REQUIRED)
    find_package(PNG REQUIRED)
    find_package(WebP REQUIRED)

    # ARM64 specific fixes
    if(IS_ARM64_BUILD)
        message(STATUS "Applying ARM64-specific fixes")
        
        # Simplify library variables to avoid complex generator expressions
        if(TARGET JPEG::JPEG)
            set(JPEG_LIBRARIES JPEG::JPEG)
        endif()

        if(TARGET PNG::PNG)
            set(PNG_LIBRARIES PNG::PNG)
        endif()

        # Fix ICU::data target which might not exist for ARM64
        if(NOT TARGET ICU::data AND TARGET ICU::uc)
            add_library(ICU::data INTERFACE IMPORTED)
            message(STATUS "Created placeholder ICU::data target for ARM64")
        endif()

        # Use x86 OpenGL headers if ARM64 headers are missing
        set(X86_HEADERS_PATH "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/x86-windows/include")
        if(EXISTS "${X86_HEADERS_PATH}/GLES3")
            include_directories("${X86_HEADERS_PATH}")
            message(STATUS "Using x86 OpenGL headers for ARM64 build at: ${X86_HEADERS_PATH}")
        else()
            # Try x64 headers as fallback
            set(X64_HEADERS_PATH "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/x64-windows/include")
            if(EXISTS "${X64_HEADERS_PATH}/GLES3")
                include_directories("${X64_HEADERS_PATH}")
                message(STATUS "Using x64 OpenGL headers for ARM64 build at: ${X64_HEADERS_PATH}")
            else()
                message(WARNING "No compatible OpenGL headers found for ARM64 build")
            endif()
        endif()
        
        # Additional ARM64 compiler flags if needed
        target_compile_options(
            mbgl-compiler-options
            INTERFACE
                $<$<COMPILE_LANGUAGE:CXX>:/bigobj>  # Handle large object files on ARM64
        )
    endif()

    find_path(DLFCN_INCLUDE_DIRS dlfcn.h)
    find_path(LIBUV_INCLUDE_DIRS uv.h)
    
    # Rest of the file remains the same...
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
    pkg_search_module(LIBUV libuv REQUIRED)
    pkg_search_module(CURL libcurl REQUIRED)
else()
    message(FATAL_ERROR "Unsupported build system: " ${CMAKE_SYSTEM_NAME})
endif()

# Continue with the rest of your existing code...
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
            ${PROJECT_SOURCE_DIR}/platform/windows/src/headless_backend_egl.cpp
            ${PROJECT_SOURCE_DIR}/platform/windows/src/gl_functions.cpp
    )
    target_compile_definitions(
        mbgl-core
        PRIVATE
            KHRONOS_STATIC
    )
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
        ${PROJECT_SOURCE_DIR}/platform/windows/include
        ${CURL_INCLUDE_DIRS}
        ${DLFCN_INCLUDE_DIRS}
        ${JPEG_INCLUDE_DIRS}
        ${LIBUV_INCLUDE_DIRS}
        ${WEBP_INCLUDE_DIRS}
)

include(${PROJECT_SOURCE_DIR}/vendor/nunicode.cmake)
include(${PROJECT_SOURCE_DIR}/vendor/sqlite.cmake)

if(NOT ${ICU_FOUND} OR "${ICU_VERSION}" VERSION_LESS 62.0 OR MLN_USE_BUILTIN_ICU)
    message(STATUS "ICU not found, too old or MLN_USE_BUILTIN_ICU requestd, using builtin.")

    set(MLN_USE_BUILTIN_ICU TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)

    set_source_files_properties(
        ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/i18n/number_format.cpp
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
            dlfcn-win32::dl
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
        $<$<BOOL:${MLN_USE_BUILTIN_ICU}>:$<IF:$<BOOL:${MLN_CORE_INCLUDE_DEPS}>,$<TARGET_OBJECTS:mbgl-vendor-icu>,mbgl-vendor-icu>>
        PNG::PNG
        mbgl-vendor-nunicode
        mbgl-vendor-sqlite
)

add_subdirectory(${PROJECT_SOURCE_DIR}/bin)
add_subdirectory(${PROJECT_SOURCE_DIR}/expression-test)
if(MLN_WITH_GLFW)
    add_subdirectory(${PROJECT_SOURCE_DIR}/platform/glfw)
endif()
if(MLN_WITH_NODE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/platform/node)
elseif(MSVC)
    target_link_libraries(
        mbgl-core
        PRIVATE
            $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    )
elseif(MSYS)
    target_link_libraries(
        mbgl-core
        PRIVATE
            ${LIBUV_LIBRARIES}
    )
endif()

add_executable(
    mbgl-test-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/test/main.cpp
)

target_compile_definitions(
    mbgl-test-runner
    PRIVATE
        MBGL_BUILDING_LIB
        WORK_DIRECTORY=${PROJECT_SOURCE_DIR}
)

if (DEFINED ENV{CI})
    target_compile_definitions(
        mbgl-test-runner
        PRIVATE
            CI_BUILD=1
    )
endif()

target_include_directories(
    mbgl-test-runner
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/windows/include
)

target_link_libraries(
    mbgl-test-runner
    PRIVATE
        mbgl-compiler-options
        $<LINK_LIBRARY:WHOLE_ARCHIVE,mbgl-test>
)

if(MSVC)
    target_link_libraries(
        mbgl-test-runner
        PRIVATE
            $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    )
endif()

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

if(MSVC)
    target_link_libraries(
        mbgl-benchmark-runner
        PRIVATE
            $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    )
endif()

add_executable(
    mbgl-render-test-runner
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/render-test/main.cpp
)

target_link_libraries(
    mbgl-render-test-runner
    PRIVATE
        mbgl-compiler-options
        mbgl-render-test
)

if(MSVC)
    target_link_libraries(
        mbgl-render-test-runner
        PRIVATE
            $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    )
endif()

if(MSVC)
    target_link_libraries(
        mbgl-expression-test
        PRIVATE
            $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    )
endif()

# Disable benchmarks in CI as they run in VM environment
if(NOT DEFINED ENV{CI})
    add_test(NAME mbgl-benchmark-runner COMMAND mbgl-benchmark-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
add_test(NAME mbgl-test-runner COMMAND mbgl-test-runner WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

install(TARGETS mbgl-render-test-runner RUNTIME DESTINATION bin)
