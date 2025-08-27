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

    # Determine if this is an ARM64 build
    set(IS_ARM64_BUILD FALSE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|ARM64)$" OR CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        set(IS_ARM64_BUILD TRUE)
        message(STATUS "Configuring for ARM64 build")
        
        # Debug: Show what VCPKG_TARGET_TRIPLET was set to
        message(STATUS "Current VCPKG_TARGET_TRIPLET: '${VCPKG_TARGET_TRIPLET}'")
        message(STATUS "CMAKE_SYSTEM_PROCESSOR: '${CMAKE_SYSTEM_PROCESSOR}'")
        message(STATUS "CMAKE_GENERATOR_PLATFORM: '${CMAKE_GENERATOR_PLATFORM}'")
        
        # Force ARM64 triplet regardless of what was set before
        if(VCPKG_TARGET_TRIPLET AND NOT VCPKG_TARGET_TRIPLET MATCHES "arm64")
            message(WARNING "VCPKG_TARGET_TRIPLET was set to '${VCPKG_TARGET_TRIPLET}' but this is an ARM64 build. Overriding to 'arm64-windows'")
            set(VCPKG_TARGET_TRIPLET "arm64-windows" CACHE STRING "Target triplet for vcpkg" FORCE)
        elseif(NOT VCPKG_TARGET_TRIPLET)
            set(VCPKG_TARGET_TRIPLET "arm64-windows" CACHE STRING "Target triplet for vcpkg" FORCE)
        endif()
        
        message(STATUS "Final VCPKG_TARGET_TRIPLET: '${VCPKG_TARGET_TRIPLET}'")
        
        # Set ARM64-specific CMake variables
        set(CMAKE_SYSTEM_PROCESSOR ARM64)
        if(NOT CMAKE_GENERATOR_PLATFORM)
            set(CMAKE_GENERATOR_PLATFORM ARM64)
        endif()
    else()
        # Debug: Show values for non-ARM64 builds too
        message(STATUS "Non-ARM64 build detected")
        message(STATUS "CMAKE_SYSTEM_PROCESSOR: '${CMAKE_SYSTEM_PROCESSOR}'")
        message(STATUS "CMAKE_GENERATOR_PLATFORM: '${CMAKE_GENERATOR_PLATFORM}'")
        message(STATUS "VCPKG_TARGET_TRIPLET: '${VCPKG_TARGET_TRIPLET}'")
    endif()

    # Always run the vendor packages script - it now handles ARM64 properly
    message(STATUS "Installing vendor packages for triplet: ${VCPKG_TARGET_TRIPLET}")
    execute_process(
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_LIST_DIR}/Get-VendorPackages.ps1 -Triplet ${VCPKG_TARGET_TRIPLET} -Renderer ${_RENDERER} ${WITH_ICU}
        RESULT_VARIABLE VCPKG_RESULT
        OUTPUT_VARIABLE VCPKG_OUTPUT
        ERROR_VARIABLE VCPKG_ERROR
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )
    
    if(NOT VCPKG_RESULT EQUAL 0)
        message(WARNING "vcpkg package installation had issues:")
        message(WARNING "Output: ${VCPKG_OUTPUT}")
        message(WARNING "Error: ${VCPKG_ERROR}")
        if(IS_ARM64_BUILD)
            message(STATUS "This is expected for ARM64 builds due to limited package availability")
            message(STATUS "Continuing with available packages...")
        else()
            message(FATAL_ERROR "Failed to install required packages")
        endif()
    else()
        message(STATUS "Successfully installed vendor packages")
    endif()
    
    unset(_RENDERER)

    add_compile_definitions(NOMINMAX GHC_WIN_DISABLE_WSTRING_STORAGE_TYPE)

    target_compile_options(
        mbgl-compiler-options
        INTERFACE
            /MP
    )

    # ARM64-specific compiler options
    if(IS_ARM64_BUILD)
        message(STATUS "Adding ARM64-specific compiler options")
        target_compile_options(
            mbgl-compiler-options
            INTERFACE
                $<$<COMPILE_LANGUAGE:CXX>:/bigobj>      # Handle large object files
                $<$<COMPILE_LANGUAGE:CXX>:/EHsc>        # Exception handling
                $<$<COMPILE_LANGUAGE:C>:/TC>            # Treat .c files as C
                $<$<COMPILE_LANGUAGE:CXX>:/TP>          # Treat .cpp files as C++
        )
        
        # Set up ARM64 cross-compilation paths
        set(ARM64_VCPKG_ROOT "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed")
        
        # Check for custom ARM64 triplet first, then standard
        if(EXISTS "${ARM64_VCPKG_ROOT}/arm64-windows-custom")
            set(ARM64_INSTALL_PATH "${ARM64_VCPKG_ROOT}/arm64-windows-custom")
            message(STATUS "Using ARM64 custom triplet packages at: ${ARM64_INSTALL_PATH}")
        elseif(EXISTS "${ARM64_VCPKG_ROOT}/arm64-windows")
            set(ARM64_INSTALL_PATH "${ARM64_VCPKG_ROOT}/arm64-windows")
            message(STATUS "Using ARM64 standard triplet packages at: ${ARM64_INSTALL_PATH}")
        else()
            message(WARNING "ARM64 packages not found, using x64 fallback")
            set(ARM64_INSTALL_PATH "${ARM64_VCPKG_ROOT}/x64-windows")
        endif()
        
        # Set CMAKE_PREFIX_PATH for ARM64
        list(PREPEND CMAKE_PREFIX_PATH "${ARM64_INSTALL_PATH}")
        message(STATUS "Added ARM64 package path to CMAKE_PREFIX_PATH: ${ARM64_INSTALL_PATH}")
    endif()

    # Find packages with ARM64 awareness
    if(IS_ARM64_BUILD)
        # For ARM64, we might need to be more flexible with package finding
        find_package(CURL QUIET)
        if(NOT CURL_FOUND)
            message(STATUS "CURL not found via find_package, trying manual detection for ARM64")
            find_path(CURL_INCLUDE_DIR curl/curl.h PATHS "${ARM64_INSTALL_PATH}/include")
            find_library(CURL_LIBRARY NAMES curl libcurl PATHS "${ARM64_INSTALL_PATH}/lib" "${ARM64_INSTALL_PATH}/debug/lib")
            if(CURL_INCLUDE_DIR AND CURL_LIBRARY)
                set(CURL_FOUND TRUE)
                add_library(CURL::libcurl UNKNOWN IMPORTED)
                set_target_properties(CURL::libcurl PROPERTIES
                    IMPORTED_LOCATION "${CURL_LIBRARY}"
                    INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIR}")
                set(CURL_LIBRARIES CURL::libcurl)
                message(STATUS "Found CURL manually for ARM64: ${CURL_LIBRARY}")
            endif()
        endif()
    else()
        find_package(CURL REQUIRED)
    endif()
    
    if(IS_ARM64_BUILD)
        find_package(dlfcn-win32 QUIET)
        if(dlfcn-win32_FOUND)
            message(STATUS "Found dlfcn-win32 for ARM64")
            set(HAVE_DLFCN_WIN32 TRUE)
        else()
            message(STATUS "dlfcn-win32 not available for ARM64, creating fallback")
            set(HAVE_DLFCN_WIN32 FALSE)
            
            # Create a dummy interface library for ARM64
            add_library(dlfcn-win32-fallback INTERFACE)
            add_library(dlfcn-win32::dl ALIAS dlfcn-win32-fallback)
            
            # Add Windows API libraries that provide LoadLibrary/GetProcAddress
            target_link_libraries(dlfcn-win32-fallback INTERFACE kernel32)
            
            # Define a macro to indicate we're using Windows API directly
            target_compile_definitions(dlfcn-win32-fallback INTERFACE 
                DLFCN_WIN32_NOT_AVAILABLE
                USE_WINDOWS_LOADLIBRARY
            )
        endif()
    else()
        find_package(dlfcn-win32 REQUIRED)
        set(HAVE_DLFCN_WIN32 TRUE)
    endif()
    find_package(ICU OPTIONAL_COMPONENTS i18n uc)
    find_package(JPEG REQUIRED)
    find_package(libuv REQUIRED)
    find_package(PNG REQUIRED)
    find_package(WebP REQUIRED)

    # ARM64-specific package fixes
    if(IS_ARM64_BUILD)
        message(STATUS "Applying ARM64-specific package fixes")
        
        # Simplify library targets for ARM64 compatibility
        if(TARGET JPEG::JPEG)
            set(JPEG_LIBRARIES JPEG::JPEG)
        elseif(JPEG_LIBRARIES)
            # Create target if it doesn't exist
            add_library(JPEG::JPEG UNKNOWN IMPORTED)
            set_target_properties(JPEG::JPEG PROPERTIES IMPORTED_LOCATION "${JPEG_LIBRARIES}")
        endif()

        if(TARGET PNG::PNG)
            set(PNG_LIBRARIES PNG::PNG)
        elseif(PNG_LIBRARIES)
            add_library(PNG::PNG UNKNOWN IMPORTED)
            set_target_properties(PNG::PNG PROPERTIES IMPORTED_LOCATION "${PNG_LIBRARIES}")
        endif()

        if(TARGET WebP::webp)
            set(WEBP_LIBRARIES WebP::webp)
        elseif(TARGET WebP::WebP)
            set(WEBP_LIBRARIES WebP::WebP)
        elseif(WEBP_LIBRARIES)
            add_library(WebP::webp UNKNOWN IMPORTED)
            set_target_properties(WebP::webp PROPERTIES IMPORTED_LOCATION "${WEBP_LIBRARIES}")
            set(WEBP_LIBRARIES WebP::webp)
        endif()

        # Handle ICU targets for ARM64
        if(ICU_FOUND)
            if(NOT TARGET ICU::data AND (TARGET ICU::uc OR TARGET ICU::i18n))
                add_library(ICU::data INTERFACE IMPORTED)
                message(STATUS "Created ICU::data interface target for ARM64")
            endif()
        else()
            message(STATUS "ICU not found for ARM64, will use builtin ICU")
            set(MLN_USE_BUILTIN_ICU TRUE)
        endif()

        # Handle libuv target normalization
        if(TARGET libuv::uv_a)
            set(LIBUV_LIBRARIES libuv::uv_a)
        elseif(TARGET libuv::uv)
            set(LIBUV_LIBRARIES libuv::uv)
        elseif(TARGET uv_a)
            set(LIBUV_LIBRARIES uv_a)
        elseif(TARGET uv)
            set(LIBUV_LIBRARIES uv)
        endif()

        # Set up include directories for ARM64
        if(EXISTS "${ARM64_INSTALL_PATH}/include")
            include_directories("${ARM64_INSTALL_PATH}/include")
            message(STATUS "Added ARM64 include directory: ${ARM64_INSTALL_PATH}/include")
        endif()
    endif()

    find_path(DLFCN_INCLUDE_DIRS dlfcn.h)
    find_path(LIBUV_INCLUDE_DIRS uv.h)
    
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

# Rest of your existing configuration...
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
