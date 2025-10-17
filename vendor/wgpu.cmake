# wgpu-native WebGPU Implementation Configuration

if(TARGET mbgl-vendor-wgpu)
    return()
endif()

if(NOT MLN_WITH_WEBGPU)
    return()
endif()

message(STATUS "Configuring wgpu-native WebGPU implementation")

set(_mln_wgpu_source_dir "${PROJECT_SOURCE_DIR}/vendor/wgpu-native")

# Check if wgpu-native exists
if(NOT EXISTS "${_mln_wgpu_source_dir}/ffi/webgpu-headers/webgpu.h")
    message(FATAL_ERROR
        "wgpu-native not found at ${_mln_wgpu_source_dir}. "
        "Please ensure the wgpu-native submodule is initialized: "
        "git submodule update --init --recursive vendor/wgpu-native")
endif()

# wgpu-native provides pre-built binaries or can be built from source
# We'll look for prebuilt binaries first, then fall back to building from source

# Platform-specific library names and paths
if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(_wgpu_lib_arch "aarch64-apple-darwin")
    else()
        set(_wgpu_lib_arch "x86_64-apple-darwin")
    endif()
    set(_wgpu_lib_name "libwgpu_native.a")
    set(_wgpu_lib_suffix ".dylib")
elseif(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_wgpu_lib_arch "x86_64-pc-windows-msvc")
    else()
        set(_wgpu_lib_arch "i686-pc-windows-msvc")
    endif()
    set(_wgpu_lib_name "wgpu_native.lib")
    set(_wgpu_lib_suffix ".dll")
else() # Linux
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(_wgpu_lib_arch "aarch64-unknown-linux-gnu")
    else()
        set(_wgpu_lib_arch "x86_64-unknown-linux-gnu")
    endif()
    set(_wgpu_lib_name "libwgpu_native.a")
    set(_wgpu_lib_suffix ".so")
endif()

# Look for prebuilt wgpu-native library
set(_wgpu_lib_search_paths
    "${_mln_wgpu_source_dir}/target/release"
    "${_mln_wgpu_source_dir}/target/${_wgpu_lib_arch}/release"
    "${_mln_wgpu_source_dir}/lib"
    "${_mln_wgpu_source_dir}/build/release"
)

find_library(WGPU_LIBRARY
    NAMES wgpu_native libwgpu_native
    PATHS ${_wgpu_lib_search_paths}
    NO_DEFAULT_PATH
)

# If not found, try to build it using cargo
if(NOT WGPU_LIBRARY)
    message(STATUS "Pre-built wgpu-native library not found, attempting to build from source...")

    # Check if cargo is available
    find_program(CARGO_EXECUTABLE cargo)

    if(NOT CARGO_EXECUTABLE)
        message(FATAL_ERROR
            "wgpu-native library not found and cargo is not available to build it. "
            "Please either:\n"
            "  1. Install Rust and cargo (https://rustup.rs/), or\n"
            "  2. Download pre-built wgpu-native binaries from https://github.com/gfx-rs/wgpu-native/releases\n"
            "     and place them in ${_mln_wgpu_source_dir}/target/release/")
    endif()

    # Build wgpu-native using cargo
    message(STATUS "Building wgpu-native with cargo (this may take a few minutes)...")

    execute_process(
        COMMAND ${CARGO_EXECUTABLE} build --release
        WORKING_DIRECTORY ${_mln_wgpu_source_dir}
        RESULT_VARIABLE _cargo_result
        OUTPUT_VARIABLE _cargo_output
        ERROR_VARIABLE _cargo_error
    )

    if(NOT _cargo_result EQUAL 0)
        message(FATAL_ERROR
            "Failed to build wgpu-native:\n${_cargo_error}\n"
            "You can try building manually:\n"
            "  cd ${_mln_wgpu_source_dir} && cargo build --release")
    endif()

    # Try to find the library again
    find_library(WGPU_LIBRARY
        NAMES wgpu_native libwgpu_native
        PATHS ${_wgpu_lib_search_paths}
        NO_DEFAULT_PATH
    )

    if(NOT WGPU_LIBRARY)
        message(FATAL_ERROR "Failed to locate wgpu-native library after building")
    endif()

    message(STATUS "Successfully built wgpu-native: ${WGPU_LIBRARY}")
else()
    message(STATUS "Found wgpu-native library: ${WGPU_LIBRARY}")
endif()

# Generate WebGPU-Cpp wrapper if needed
set(_webgpu_cpp_dir "${PROJECT_SOURCE_DIR}/vendor/webgpu-cpp")
set(_webgpu_cpp_header "${_webgpu_cpp_dir}/webgpu.hpp")

if(NOT EXISTS "${_webgpu_cpp_header}")
    message(STATUS "Generating WebGPU-Cpp wrapper...")

    # Check if Python 3 is available
    find_program(PYTHON_EXECUTABLE python3 python)
    if(NOT PYTHON_EXECUTABLE)
        message(FATAL_ERROR
            "Python is required to generate WebGPU-Cpp wrapper. "
            "Please install Python 3.")
    endif()

    # Run the generator script using the local webgpu.h from wgpu-native
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} generate.py
            --use-init-macros
            --header ${_mln_wgpu_source_dir}/ffi/webgpu-headers/webgpu.h
        WORKING_DIRECTORY ${_webgpu_cpp_dir}
        RESULT_VARIABLE _gen_result
        OUTPUT_VARIABLE _gen_output
        ERROR_VARIABLE _gen_error
    )

    if(NOT _gen_result EQUAL 0)
        message(FATAL_ERROR
            "Failed to generate WebGPU-Cpp wrapper:\n${_gen_error}")
    endif()

    message(STATUS "Successfully generated WebGPU-Cpp wrapper")
endif()

# Create compatibility shims for Dawn header paths if they don't exist
set(_compat_shim_dir "${_webgpu_cpp_dir}/wgpu-native/webgpu")
set(_compat_shim_header "${_compat_shim_dir}/webgpu_cpp.h")
set(_compat_shim_webgpu_h "${_compat_shim_dir}/webgpu.h")

if(NOT EXISTS "${_compat_shim_header}")
    message(STATUS "Creating WebGPU-Cpp compatibility shim...")

    file(MAKE_DIRECTORY "${_compat_shim_dir}")
    file(WRITE "${_compat_shim_header}"
"#pragma once
// Compatibility shim for Dawn header paths
// This file allows MapLibre code to use #include <webgpu/webgpu_cpp.h>
// for both Dawn and wgpu-native backends

// Include the WebGPU-Cpp wrapper
#include \"../../webgpu.hpp\"
")

    message(STATUS "Successfully created WebGPU-Cpp compatibility shim")
endif()

if(NOT EXISTS "${_compat_shim_webgpu_h}")
    message(STATUS "Creating webgpu.h compatibility shim...")

    file(WRITE "${_compat_shim_webgpu_h}"
"#pragma once
// Compatibility shim for Dawn header paths
// This file allows MapLibre code to use #include <webgpu/webgpu.h>
// for both Dawn and wgpu-native backends

// Include the actual webgpu.h header
#include <webgpu.h>
")

    message(STATUS "Successfully created webgpu.h compatibility shim")
endif()

# Also create wgpu.h shim for wgpu-native specific includes
set(_compat_shim_wgpu_h "${_compat_shim_dir}/wgpu.h")
if(NOT EXISTS "${_compat_shim_wgpu_h}")
    message(STATUS "Creating wgpu.h compatibility shim...")

    file(WRITE "${_compat_shim_wgpu_h}"
"#pragma once
// Compatibility shim for wgpu-native specific header
// This file allows MapLibre code to use #include <webgpu/wgpu.h>
// for both Dawn and wgpu-native backends

// Include the actual wgpu.h header
#include <wgpu.h>
")

    message(STATUS "Successfully created wgpu.h compatibility shim")
endif()

# Create interface library
add_library(mbgl-vendor-wgpu INTERFACE)

target_link_libraries(mbgl-vendor-wgpu INTERFACE ${WGPU_LIBRARY})

# Add include directories for webgpu.h, wgpu.h, and C++ wrapper
target_include_directories(mbgl-vendor-wgpu
    SYSTEM INTERFACE
        ${_webgpu_cpp_dir}                      # For generated webgpu.hpp
        ${_webgpu_cpp_dir}/wgpu-native          # For webgpu/webgpu_cpp.h compatibility shim
        ${_mln_wgpu_source_dir}/ffi/webgpu-headers
        ${_mln_wgpu_source_dir}/ffi
)

# Platform-specific system libraries that wgpu-native needs
if(APPLE)
    target_link_libraries(mbgl-vendor-wgpu INTERFACE
        "-framework Foundation"
        "-framework QuartzCore"
        "-framework Metal"
    )
elseif(WIN32)
    target_link_libraries(mbgl-vendor-wgpu INTERFACE
        d3d12
        dxgi
        d3dcompiler
        ws2_32
        userenv
        bcrypt
        ntdll
    )
else() # Linux
    # wgpu-native on Linux might need these depending on the build configuration
    find_package(Threads REQUIRED)
    target_link_libraries(mbgl-vendor-wgpu INTERFACE
        Threads::Threads
        ${CMAKE_DL_LIBS}
    )
endif()

# Suppress warnings from wgpu headers
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(mbgl-vendor-wgpu INTERFACE
        -Wno-strict-aliasing
        -Wno-error=strict-aliasing
    )
endif()

set_target_properties(
    mbgl-vendor-wgpu
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "wgpu-native"
        INTERFACE_MAPLIBRE_URL "https://github.com/gfx-rs/wgpu-native"
        INTERFACE_MAPLIBRE_AUTHOR "gfx-rs developers"
        INTERFACE_MAPLIBRE_LICENSE "${PROJECT_SOURCE_DIR}/vendor/wgpu-native/LICENSE.MIT"
)
