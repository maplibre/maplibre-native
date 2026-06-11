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
    find_package(dlfcn-win32 REQUIRED)
    find_package(ICU OPTIONAL_COMPONENTS i18n uc)
    find_package(JPEG REQUIRED)
    find_package(libuv REQUIRED)
    find_package(PNG REQUIRED)
    find_package(WebP REQUIRED)
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
        $<$<BOOL:${MLN_USE_BUILTIN_ICU}>:mbgl-vendor-icu>
        PNG::PNG
        mbgl-vendor-nunicode
        mbgl-vendor-sqlite
)

if(MLN_CREATE_AMALGAMATION)
    if(MSVC)
        message(FATAL_ERROR "MLN_CREATE_AMALGAMATION=ON only supports on MSYS2/MinGW (not MSVC)")
    endif()

    if(NOT MINGW)
        message(FATAL_ERROR "MLN_CREATE_AMALGAMATION=ON requires MSYS2/MinGW toolchain on Windows")
    endif()

    # ---- Locate tools robustly (CMake cannot use MSYS2 '/mingw64/...' paths directly) ----
    set(_mln_tool_paths "")

    # Derive MSYS2 root from the running CMake executable path.
    # Example: CMAKE_COMMAND = C:/msys64/clang64/bin/cmake.exe
    #   -> msys root: C:/msys64
    get_filename_component(_mln_cmake_bindir "${CMAKE_COMMAND}" DIRECTORY)
    get_filename_component(_mln_prefix_dir "${_mln_cmake_bindir}" DIRECTORY)
    get_filename_component(_mln_msys_root "${_mln_prefix_dir}" DIRECTORY)

    list(APPEND _mln_tool_paths "${_mln_msys_root}/mingw64/bin")

    # Extra fallback: ask cygpath for a Windows/mixed path to /mingw64/bin
    find_program(_mln_cygpath NAMES cygpath cygpath.exe)
    if(_mln_cygpath)
        execute_process(
            COMMAND "${_mln_cygpath}" -m /mingw64/bin
            OUTPUT_VARIABLE _mln_mingw64_bin_m
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(_mln_mingw64_bin_m)
            list(APPEND _mln_tool_paths "${_mln_mingw64_bin_m}")
        endif()
    endif()

    list(REMOVE_DUPLICATES _mln_tool_paths)

    # GNU ld.bfd is required for relocatable/partial link on Windows COFF.
    find_program(MLN_LD_BFD NAMES ld.bfd.exe ld.bfd)
    if(NOT MLN_LD_BFD)
        find_program(MLN_LD_BFD NAMES ld.bfd.exe ld.bfd PATHS ${_mln_tool_paths} NO_DEFAULT_PATH)
    endif()

    # GNU objcopy is required (llvm-objcopy does not support --keep-global-symbol(s) for COFF in this flow).
    find_program(MLN_GNU_OBJCOPY NAMES objcopy.exe objcopy PATHS ${_mln_tool_paths} NO_DEFAULT_PATH)
    if(NOT MLN_GNU_OBJCOPY)
        find_program(MLN_GNU_OBJCOPY NAMES objcopy.exe objcopy)
    endif()

    find_program(MLN_AR NAMES llvm-ar.exe llvm-ar ar.exe ar)
    find_program(MLN_RANLIB NAMES llvm-ranlib.exe llvm-ranlib ranlib.exe ranlib)

    if(NOT MLN_LD_BFD)
        message(FATAL_ERROR "ld.bfd required when MLN_CREATE_AMALGAMATION=ON. Tried: ${_mln_tool_paths} (install mingw-w64-x86_64-binutils)")
    endif()
    if(NOT MLN_GNU_OBJCOPY)
        message(FATAL_ERROR "GNU objcopy required when MLN_CREATE_AMALGAMATION=ON. Tried: ${_mln_tool_paths} (install mingw-w64-x86_64-binutils)")
    endif()
    if(NOT MLN_AR)
        message(FATAL_ERROR "ar required when MLN_CREATE_AMALGAMATION=ON")
    endif()
    if(NOT MLN_RANLIB)
        message(FATAL_ERROR "ranlib required when MLN_CREATE_AMALGAMATION=ON")
    endif()

    execute_process(
        COMMAND "${MLN_GNU_OBJCOPY}" --version
        OUTPUT_VARIABLE _mln_objcopy_version
        ERROR_VARIABLE _mln_objcopy_version
    )
    if(NOT _mln_objcopy_version MATCHES "GNU objcopy")
        message(FATAL_ERROR "GNU objcopy is required (do not use llvm-objcopy for COFF here). Found: ${MLN_GNU_OBJCOPY}")
    endif()

    message(STATUS "Found ld.bfd: ${MLN_LD_BFD}")
    message(STATUS "Found GNU objcopy: ${MLN_GNU_OBJCOPY}")
    message(STATUS "Found ar: ${MLN_AR}")
    message(STATUS "Found ranlib: ${MLN_RANLIB}")

    include(${PROJECT_SOURCE_DIR}/cmake/find_static_library.cmake)

    # ---- Collect static libraries to fold into the amalgamation ----
    set(STATIC_LIBS "")
    find_static_library(STATIC_LIBS NAMES png)
    find_static_library(STATIC_LIBS NAMES z)
    find_static_library(STATIC_LIBS NAMES jpeg)
    find_static_library(STATIC_LIBS NAMES webp)
    find_static_library(STATIC_LIBS NAMES uv uv_a)
    find_static_library(STATIC_LIBS NAMES curl)

    # ICU (system static libs in MSYS2 CLANG64)
    set(ICU_LIBS "")
    find_static_library(ICU_LIBS NAMES icuin)
    find_static_library(ICU_LIBS NAMES icuuc)
    find_static_library(ICU_LIBS NAMES icudt)

    # Optional deps (do not fail configure if missing)
    set(OPTIONAL_STATIC_LIBS "")
    set(_old_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    foreach(_mln_opt ssl crypto bz2 bzip2 zstd nghttp2 brotlidec brotlicommon brotlienc cares ssh2 idn2)
        unset(_mln_found_lib CACHE)
        find_library(_mln_found_lib NAMES ${_mln_opt})
        if(_mln_found_lib)
            message(STATUS "find_static_library: Found optional static [${_mln_opt}] -> ${_mln_found_lib}")
            list(APPEND OPTIONAL_STATIC_LIBS ${_mln_found_lib})
        endif()
    endforeach()
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_old_suffixes})
    unset(_mln_opt)
    unset(_mln_found_lib CACHE)

    # Vulkan-only libs (OpenGL preset keeps this empty)
    set(VULKAN_LIBS "")
    if(MLN_WITH_VULKAN)
        list(APPEND VULKAN_LIBS
            $<TARGET_FILE:glslang>
            $<TARGET_FILE:SPIRV>
            $<TARGET_FILE:MachineIndependent>
            $<TARGET_FILE:GenericCodeGen>
            $<TARGET_FILE:glslang-default-resource-limits>
        )
    endif()

    # Response file content: must NOT use bracket quotes here because we need ${...} expanded.
    set(MLN_AMALGAM_RSP "${CMAKE_CURRENT_BINARY_DIR}/mbgl-core-amalgam.rsp")

    set(_mln_rsp_content "")
    string(APPEND _mln_rsp_content "--relocatable\n")
    string(APPEND _mln_rsp_content "-o\n")
    string(APPEND _mln_rsp_content "mbgl-core-amalgam-merged.o\n")
    string(APPEND _mln_rsp_content "--whole-archive\n")

    # Target archives (generator expressions are OK; file(GENERATE) will evaluate them)
    string(APPEND _mln_rsp_content "$<TARGET_FILE:mbgl-core>\n")
    string(APPEND _mln_rsp_content "$<TARGET_FILE:mbgl-freetype>\n")
    string(APPEND _mln_rsp_content "$<TARGET_FILE:mbgl-vendor-csscolorparser>\n")
    string(APPEND _mln_rsp_content "$<TARGET_FILE:mbgl-harfbuzz>\n")
    string(APPEND _mln_rsp_content "$<TARGET_FILE:mbgl-vendor-nunicode>\n")
    string(APPEND _mln_rsp_content "$<TARGET_FILE:mbgl-vendor-sqlite>\n")
    string(APPEND _mln_rsp_content "$<TARGET_FILE:mbgl-vendor-parsedate>\n")

    # These are plain strings (absolute .a paths); they MUST be expanded now.
    if(MLN_ICU_LIBS_RSP)
        string(APPEND _mln_rsp_content "${MLN_ICU_LIBS_RSP}\n")
    endif()
    if(MLN_VULKAN_LIBS_RSP)
        string(APPEND _mln_rsp_content "${MLN_VULKAN_LIBS_RSP}\n")
    endif()
    if(MLN_STATIC_LIBS_RSP)
        string(APPEND _mln_rsp_content "${MLN_STATIC_LIBS_RSP}\n")
    endif()
    if(MLN_OPTIONAL_STATIC_LIBS_RSP)
        string(APPEND _mln_rsp_content "${MLN_OPTIONAL_STATIC_LIBS_RSP}\n")
    endif()

    string(APPEND _mln_rsp_content "--no-whole-archive\n")

    file(GENERATE
        OUTPUT "${MLN_AMALGAM_RSP}"
        CONTENT "${_mln_rsp_content}"
    )

    # OpenGL goal name (Vulkan is kept for later; harmless under windows-opengl preset)
    if(MLN_WITH_VULKAN)
        set(MLN_AMALGAM_FINAL_NAME "libmaplibre-native-core-amalgam-windowx-x64-vulkan.a")
    else()
        set(MLN_AMALGAM_FINAL_NAME "libmaplibre-native-core-amalgam-windowx-x64-opengl.a")
    endif()

    add_custom_command(
        TARGET mbgl-core
        POST_BUILD
        BYPRODUCTS
            "${CMAKE_CURRENT_BINARY_DIR}/mbgl-core-amalgam-merged.o"
            "${CMAKE_CURRENT_BINARY_DIR}/mbgl-core-amalgam-merged-public.o"
            "${CMAKE_CURRENT_BINARY_DIR}/libmbgl-core-amalgam.a"
            "${CMAKE_CURRENT_BINARY_DIR}/${MLN_AMALGAM_FINAL_NAME}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E rm -f
            "mbgl-core-amalgam-merged.o"
            "mbgl-core-amalgam-merged-public.o"
            "libmbgl-core-amalgam.a"
            "${MLN_AMALGAM_FINAL_NAME}"
        COMMAND "${MLN_LD_BFD}" "@${MLN_AMALGAM_RSP}"
        COMMAND "${MLN_GNU_OBJCOPY}" --wildcard -G "*mbgl*"
            "mbgl-core-amalgam-merged.o"
            "mbgl-core-amalgam-merged-public.o"
        COMMAND "${MLN_AR}" rcs "libmbgl-core-amalgam.a" "mbgl-core-amalgam-merged-public.o"
        COMMAND "${MLN_RANLIB}" "libmbgl-core-amalgam.a"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "libmbgl-core-amalgam.a"
            "${MLN_AMALGAM_FINAL_NAME}"
        VERBATIM
    )
endif()

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
