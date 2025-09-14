cmake_minimum_required(VERSION 3.24)

# Find GLFW if needed
if(MLN_WITH_GLFW)
    if(WIN32)
        find_package(glfw3 REQUIRED)
    elseif(APPLE)
        # On macOS with Homebrew, GLFW is typically here
        set(GLFW_ROOT "/opt/homebrew/opt/glfw")
        find_package(glfw3 3.3 REQUIRED PATHS ${GLFW_ROOT}/lib/cmake/glfw3 NO_DEFAULT_PATH)
        set(GLFW_LIBRARIES glfw)
    else()
        find_package(PkgConfig REQUIRED)
        pkg_search_module(GLFW glfw3 REQUIRED)
    endif()
endif()

# WebGPU Example
if(MLN_WITH_WEBGPU AND MLN_WITH_GLFW)
    add_executable(webgpu_example webgpu_example.cpp)

    target_link_libraries(
        webgpu_example
        PRIVATE
            mbgl-core
            ${GLFW_LIBRARIES}
    )

    target_include_directories(
        webgpu_example
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
    )

    target_compile_definitions(
        webgpu_example
        PRIVATE
            MLN_RENDER_BACKEND_WEBGPU=1
    )

    # Platform-specific configuration
    if(APPLE)
        target_link_libraries(
            webgpu_example
            PRIVATE
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo"
                "-framework Metal"
                "-framework QuartzCore"
        )
    elseif(WIN32)
        target_link_libraries(
            webgpu_example
            PRIVATE
                d3d12.lib
                dxgi.lib
                d3dcompiler.lib
        )
    else()
        # Linux
        find_package(X11 REQUIRED)
        target_link_libraries(
            webgpu_example
            PRIVATE
                ${X11_LIBRARIES}
        )
    endif()

    # Copy the executable to a more convenient location
    add_custom_command(
        TARGET webgpu_example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:webgpu_example>
            ${CMAKE_BINARY_DIR}/bin/webgpu_example
    )

    message(STATUS "WebGPU example will be built")
endif()

# GLFW + WebGPU Example
if(MLN_WITH_WEBGPU AND MLN_WITH_GLFW)
    if(APPLE)
        add_executable(webgpu_glfw_example webgpu_glfw_example.mm)
    else()
        add_executable(webgpu_glfw_example webgpu_glfw_example.cpp)
    endif()

    target_link_libraries(
        webgpu_glfw_example
        PRIVATE
            mbgl-core
            ${GLFW_LIBRARIES}
    )

    target_include_directories(
        webgpu_glfw_example
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
    )

    target_compile_definitions(
        webgpu_glfw_example
        PRIVATE
            MLN_RENDER_BACKEND_WEBGPU=1
    )

    # Platform-specific configuration
    if(APPLE)
        target_link_libraries(
            webgpu_glfw_example
            PRIVATE
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo"
                "-framework Metal"
                "-framework QuartzCore"
        )
    elseif(WIN32)
        target_link_libraries(
            webgpu_glfw_example
            PRIVATE
                d3d12.lib
                dxgi.lib
                d3dcompiler.lib
        )
    else()
        # Linux
        find_package(X11 REQUIRED)
        target_link_libraries(
            webgpu_glfw_example
            PRIVATE
                ${X11_LIBRARIES}
        )
    endif()

    # Copy the executable to a more convenient location
    add_custom_command(
        TARGET webgpu_glfw_example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:webgpu_glfw_example>
            ${CMAKE_BINARY_DIR}/bin/webgpu_glfw_example
    )

    message(STATUS "WebGPU GLFW example will be built")
endif()

# Standalone GLFW example (no MapLibre dependencies)
if(MLN_WITH_GLFW)
    add_executable(webgpu_glfw_standalone webgpu_glfw_standalone.cpp)

    target_link_libraries(
        webgpu_glfw_standalone
        PRIVATE
            ${GLFW_LIBRARIES}
    )

    target_include_directories(
        webgpu_glfw_standalone
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
    )

    # Platform-specific configuration
    if(APPLE)
        target_link_libraries(
            webgpu_glfw_standalone
            PRIVATE
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo"
        )
    endif()

    message(STATUS "WebGPU GLFW standalone example will be built")

    # Simple Dawn WebGPU demo
    if(MLN_WITH_WEBGPU)
        message(STATUS "Simple Dawn WebGPU demo will be built")

        message(STATUS "Simple Dawn WebGPU demo will be built")

        add_executable(simple_dawn_webgpu simple_dawn_webgpu.cpp)

        target_link_libraries(
        simple_dawn_webgpu
        PRIVATE
            mbgl-core
    )

        target_compile_definitions(
        simple_dawn_webgpu
        PRIVATE
            MLN_WITH_DAWN=1
    )
    endif()

    # Dawn WebGPU Triangle with GLFW
    if(MLN_WITH_WEBGPU AND MLN_WITH_GLFW)
        message(STATUS "Dawn WebGPU triangle example will be built")

        if(APPLE)
            add_executable(dawn_webgpu_triangle dawn_webgpu_triangle.mm)
        else()
            add_executable(dawn_webgpu_triangle dawn_webgpu_triangle.cpp)
        endif()

        target_link_libraries(
        dawn_webgpu_triangle
        PRIVATE
            mbgl-core
            ${GLFW_LIBRARIES}
    )

        target_include_directories(
        dawn_webgpu_triangle
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
    )

        target_compile_definitions(
        dawn_webgpu_triangle
        PRIVATE
            MLN_WITH_DAWN=1
    )

        if(APPLE)
            target_link_libraries(
            dawn_webgpu_triangle
            PRIVATE
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo"
        )
        endif()
        
        # Simple WebGPU Triangle Example
        message(STATUS "Simple WebGPU triangle example will be built")
        
        add_executable(webgpu_simple_triangle webgpu_simple_triangle.cpp)
        
        target_link_libraries(
            webgpu_simple_triangle
            PRIVATE
                mbgl-core
                ${GLFW_LIBRARIES}
        )
        
        target_include_directories(
            webgpu_simple_triangle
            PRIVATE
                ${GLFW_INCLUDE_DIRS}
        )
        
        target_compile_definitions(
            webgpu_simple_triangle
            PRIVATE
                MLN_WITH_DAWN=1
                MLN_RENDER_BACKEND_WEBGPU=1
        )
        
        if(APPLE)
            target_link_libraries(
                webgpu_simple_triangle
                PRIVATE
                    "-framework Cocoa"
                    "-framework IOKit"
                    "-framework CoreVideo"
                    "-framework Metal"
                    "-framework QuartzCore"
            )
        endif()
    endif()
endif()  # Close the parent if(MLN_WITH_GLFW) block

# WebGPU Map Example
if(MLN_WITH_WEBGPU AND MLN_WITH_GLFW)
    message(STATUS "WebGPU Map example will be built")

    if(APPLE)
        add_executable(webgpu_map webgpu_map.mm)
        add_executable(webgpu_map_simple webgpu_map_simple.mm)
    else()
        add_executable(webgpu_map webgpu_map.cpp)
    endif()

    target_link_libraries(
        webgpu_map
        PRIVATE
            mbgl-core
            ${GLFW_LIBRARIES}
    )

    target_include_directories(
        webgpu_map
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
    )

    target_compile_definitions(
        webgpu_map
        PRIVATE
            MLN_WITH_DAWN=1
    )

    if(APPLE)
        target_link_libraries(
            webgpu_map
            PRIVATE
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo"
        )
    endif()
endif()

# WebGPU Render Demo
if(MLN_WITH_WEBGPU AND MLN_WITH_GLFW)
    message(STATUS "WebGPU Render Demo will be built")

    if(APPLE)
        add_executable(webgpu_render_demo webgpu_render_demo.mm)
    else()
        add_executable(webgpu_render_demo webgpu_render_demo.cpp)
    endif()

    target_link_libraries(
        webgpu_render_demo
        PRIVATE
            mbgl-core
            ${GLFW_LIBRARIES}
    )

    target_include_directories(
        webgpu_render_demo
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
    )

    target_compile_definitions(
        webgpu_render_demo
        PRIVATE
            MLN_WITH_DAWN=1
    )

    if(APPLE)
        target_link_libraries(
            webgpu_render_demo
            PRIVATE
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo"
        )
    endif()
endif()

# OpenGL triangle example (shows what WebGPU would render)
if(MLN_WITH_GLFW)
    add_executable(glfw_opengl_triangle glfw_opengl_triangle.cpp)

    find_package(OpenGL REQUIRED)

    target_link_libraries(
        glfw_opengl_triangle
        PRIVATE
            ${GLFW_LIBRARIES}
            ${OPENGL_LIBRARIES}
    )

    target_include_directories(
        glfw_opengl_triangle
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
    )

    if(APPLE)
        target_link_libraries(
            glfw_opengl_triangle
            PRIVATE
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo"
                "-framework OpenGL"
        )
    endif()

    message(STATUS "OpenGL triangle example will be built")
endif()

# Dawn WebGPU Triangle Example (with real rendering!)
if(MLN_WITH_WEBGPU AND MLN_WITH_GLFW AND EXISTS "/Users/admin/repos/dawn")
    if(APPLE)
        add_executable(webgpu_dawn_triangle webgpu_dawn_triangle.mm)
    else()
        add_executable(webgpu_dawn_triangle webgpu_dawn_triangle.cpp)
    endif()

    # Dawn paths
    set(DAWN_DIR "/Users/admin/repos/dawn")
    set(DAWN_BUILD_DIR "${DAWN_DIR}/out/Release")

    target_include_directories(webgpu_dawn_triangle
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
            ${DAWN_DIR}/include
            ${DAWN_BUILD_DIR}/gen/include
    )

    target_link_libraries(webgpu_dawn_triangle
        PRIVATE
            ${GLFW_LIBRARIES}
            ${DAWN_BUILD_DIR}/src/dawn/native/libwebgpu_dawn.a
            ${DAWN_BUILD_DIR}/src/dawn/native/libdawn_native.a
            ${DAWN_BUILD_DIR}/src/dawn/libdawn_proc.a
            ${DAWN_BUILD_DIR}/src/dawn/platform/libdawn_platform.a
            ${DAWN_BUILD_DIR}/src/dawn/common/libdawn_common.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_api.a
            ${DAWN_BUILD_DIR}/third_party/abseil-cpp/absl/base/libabsl_base.a
            ${DAWN_BUILD_DIR}/third_party/abseil-cpp/absl/base/libabsl_raw_logging_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil-cpp/absl/base/libabsl_log_severity.a
            ${DAWN_BUILD_DIR}/third_party/abseil-cpp/absl/container/libabsl_raw_hash_set.a
            ${DAWN_BUILD_DIR}/third_party/abseil-cpp/absl/container/libabsl_hashtablez_sampler.a
            ${DAWN_BUILD_DIR}/third_party/abseil-cpp/absl/numeric/libabsl_int128.a
            ${DAWN_BUILD_DIR}/third_party/abseil-cpp/absl/strings/libabsl_strings.a
            ${DAWN_BUILD_DIR}/third_party/abseil-cpp/absl/strings/libabsl_strings_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil-cpp/absl/types/libabsl_bad_optional_access.a
            "-framework Metal"
            "-framework QuartzCore"
            "-framework IOKit"
            "-framework IOSurface"
            "-framework CoreGraphics"
            "-framework Cocoa"
            "-framework CoreVideo"
    )

    target_compile_definitions(webgpu_dawn_triangle
        PRIVATE
            DAWN_ENABLE_BACKEND_METAL=1
    )

    # C++17 required for Dawn
    set_property(TARGET webgpu_dawn_triangle PROPERTY CXX_STANDARD 17)

    message(STATUS "Dawn WebGPU triangle example will be built")
endif()

# Minimal Dawn WebGPU Demo (just creates instance and device)
if(MLN_WITH_WEBGPU AND MLN_WITH_GLFW AND EXISTS "/Users/admin/repos/dawn")
    add_executable(webgpu_dawn_minimal webgpu_dawn_minimal.cpp)

    # Dawn paths
    set(DAWN_DIR "/Users/admin/repos/dawn")
    set(DAWN_BUILD_DIR "${DAWN_DIR}/out/Release")

    target_include_directories(webgpu_dawn_minimal
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
            ${DAWN_DIR}/include
            ${DAWN_BUILD_DIR}/gen/include
    )

    # Use find to get all libraries
    execute_process(
        COMMAND find ${DAWN_BUILD_DIR} -name "*.a" -type f
        OUTPUT_VARIABLE ALL_DAWN_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "\n" ";" ALL_DAWN_LIBS_LIST ${ALL_DAWN_LIBS})

    target_link_libraries(webgpu_dawn_minimal
        PRIVATE
            ${GLFW_LIBRARIES}
            ${ALL_DAWN_LIBS_LIST}
            "-framework Metal"
            "-framework QuartzCore"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework IOSurface"
            "-framework CoreVideo"
            "-framework CoreGraphics"
            "-framework Foundation"
    )

    # Only build for arm64
    set_target_properties(webgpu_dawn_minimal PROPERTIES
        OSX_ARCHITECTURES "arm64"
        CXX_STANDARD 17
    )

    target_compile_definitions(webgpu_dawn_minimal
        PRIVATE
            DAWN_ENABLE_BACKEND_METAL=1
    )

    message(STATUS "Minimal Dawn WebGPU demo will be built")
endif()

# Simple Dawn WebGPU Demo (just clears screen)
if(MLN_WITH_WEBGPU AND MLN_WITH_GLFW AND EXISTS "/Users/admin/repos/dawn")
    add_executable(webgpu_dawn_simple webgpu_dawn_simple.mm)

    # Dawn paths
    set(DAWN_DIR "/Users/admin/repos/dawn")
    set(DAWN_BUILD_DIR "${DAWN_DIR}/out/Release")

    target_include_directories(webgpu_dawn_simple
        PRIVATE
            ${GLFW_INCLUDE_DIRS}
            ${DAWN_DIR}/include
            ${DAWN_BUILD_DIR}/gen/include
    )

    # Link minimal Dawn libraries for simple demo
    target_link_libraries(webgpu_dawn_simple
        PRIVATE
            ${GLFW_LIBRARIES}
            ${DAWN_BUILD_DIR}/src/dawn/native/libdawn_native.a
            ${DAWN_BUILD_DIR}/src/dawn/platform/libdawn_platform.a
            ${DAWN_BUILD_DIR}/src/dawn/common/libdawn_common.a
            # Tint (shader compiler) libraries - order matters!
            ${DAWN_BUILD_DIR}/src/tint/libtint_api.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_api_common.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_lang_wgsl.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_lang_wgsl_sem.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_lang_wgsl_inspector.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_lang_wgsl_program.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_lang_wgsl_resolver.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_lang_wgsl_writer_raise.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_lang_core_ir.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_lang_core_ir_type.a
            # Tint utilities and base libraries
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_rtti.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_symbol.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_text.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_diagnostic.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_ice.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_math.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_memory.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_strconv.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_system.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_text_generator.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_containers.a
            ${DAWN_BUILD_DIR}/src/tint/libtint_utils_macros.a
            # Abseil dependencies - only the ones that exist
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/container/libabsl_raw_hash_set.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/container/libabsl_hashtablez_sampler.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/hash/libabsl_hash.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/hash/libabsl_city.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/strings/libabsl_strings.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/strings/libabsl_strings_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/strings/libabsl_str_format_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/strings/libabsl_cord.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/strings/libabsl_cord_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/strings/libabsl_cordz_info.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/strings/libabsl_cordz_handle.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/strings/libabsl_cordz_functions.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/crc/libabsl_crc32c.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/crc/libabsl_crc_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/crc/libabsl_crc_cpu_detect.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/crc/libabsl_crc_cord_state.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/profiling/libabsl_exponential_biased.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/base/libabsl_base.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/base/libabsl_raw_logging_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/base/libabsl_spinlock_wait.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/base/libabsl_throw_delegate.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/base/libabsl_malloc_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/numeric/libabsl_int128.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/synchronization/libabsl_synchronization.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/synchronization/libabsl_graphcycles_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/debugging/libabsl_stacktrace.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/debugging/libabsl_symbolize.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/debugging/libabsl_debugging_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/debugging/libabsl_demangle_internal.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/time/libabsl_time.a
            ${DAWN_BUILD_DIR}/third_party/abseil/absl/time/libabsl_time_zone.a
            "-framework Metal"
            "-framework QuartzCore"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework IOSurface"
            "-framework CoreVideo"
    )

    # Only build for arm64 on Apple Silicon
    set_target_properties(webgpu_dawn_simple PROPERTIES
        OSX_ARCHITECTURES "arm64"
    )

    target_compile_definitions(webgpu_dawn_simple
        PRIVATE
            DAWN_ENABLE_BACKEND_METAL=1
    )

    # C++17 required for Dawn
    set_property(TARGET webgpu_dawn_simple PROPERTY CXX_STANDARD 17)

    message(STATUS "Simple Dawn WebGPU demo will be built")
endif()


# Add more examples here as needed
