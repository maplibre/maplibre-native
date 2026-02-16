# Dawn WebGPU Implementation Configuration

if(TARGET mbgl-vendor-dawn)
    return()
endif()

if(NOT MLN_WITH_WEBGPU)
    return()
endif()

if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

set(MLN_DAWN_GIT_VERSION "v20260210.203301" CACHE STRING "Git ref (branch, tag, or commit) used when fetching Dawn")

message(STATUS "Configuring Dawn WebGPU implementation (${MLN_DAWN_GIT_VERSION})")

set(_mln_dawn_source_dir "${PROJECT_SOURCE_DIR}/vendor/dawn")
set(_mln_dawn_binary_dir "${CMAKE_BINARY_DIR}/vendor/dawn/build")


if(EXISTS "${_mln_dawn_source_dir}/CMakeLists.txt")
    set(FETCHCONTENT_SOURCE_DIR_MAPLIBRE_DAWN ${_mln_dawn_source_dir})
endif()

include(FetchContent)

FetchContent_Declare(maplibre_dawn
    GIT_REPOSITORY https://github.com/google/dawn.git
    GIT_TAG ${MLN_DAWN_GIT_VERSION}
    GIT_SUBMODULES ""
)

FetchContent_GetProperties(maplibre_dawn)
if(NOT maplibre_dawn_POPULATED)
    message(STATUS "Fetching Dawn sources into ${_mln_dawn_source_dir}")
    FetchContent_Populate(maplibre_dawn)

    if(NOT EXISTS "${_mln_dawn_source_dir}/CMakeLists.txt")
        file(REMOVE_RECURSE "${_mln_dawn_source_dir}")
        file(RENAME "${maplibre_dawn_SOURCE_DIR}" "${_mln_dawn_source_dir}")
        # Make future configure runs reuse the relocated checkout.
        set(FETCHCONTENT_SOURCE_DIR_MAPLIBRE_DAWN "${_mln_dawn_source_dir}" CACHE PATH "" FORCE)
    endif()
endif()

set(maplibre_dawn_SOURCE_DIR "${_mln_dawn_source_dir}")

set(DAWN_DIR "${_mln_dawn_source_dir}")
set(DAWN_BUILD_DIR "${_mln_dawn_binary_dir}")

# Configure Dawn the same way as the quickstart recommends.
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "Use Dawn's dependency bootstrapper" FORCE)
set(DAWN_ENABLE_INSTALL ON CACHE BOOL "Generate install targets for Dawn" FORCE)
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "Disable Dawn samples" FORCE)
set(DAWN_BUILD_TESTS OFF CACHE BOOL "Disable Dawn tests" FORCE)
set(DAWN_BUILD_BENCHMARKS OFF CACHE BOOL "Disable Dawn benchmarks" FORCE)
set(DAWN_BUILD_NODE_BINDINGS OFF CACHE BOOL "Disable Dawn Node bindings" FORCE)
set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "Disable Tint command line tools" FORCE)
set(DAWN_WERROR OFF CACHE BOOL "Disable -Werror in Dawn" FORCE)
set(DAWN_BUILD_PROTOBUF OFF CACHE BOOL "Disable protobuf for cross-compilation" FORCE)

# GLFW is not available on iOS (Dawn auto-detects it via UNIX AND NOT ANDROID)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(DAWN_USE_GLFW OFF CACHE BOOL "Disable GLFW on iOS" FORCE)
endif()

# The Xcode generator does not properly handle $<TARGET_OBJECTS:...> in add_library(),
# which breaks Dawn's bundle_libraries() macro. Disable the monolithic library for Xcode
# and link individual Dawn component targets instead.
if(CMAKE_GENERATOR STREQUAL "Xcode")
    set(DAWN_BUILD_MONOLITHIC_LIBRARY "OFF" CACHE STRING "" FORCE)
endif()

if(NOT TARGET dawn_native)
    if(APPLE)
        enable_language(OBJC OBJCXX)
    endif()
    add_subdirectory(${DAWN_DIR} ${DAWN_BUILD_DIR})
endif()

# Tint's MSL validator includes Metal headers which use GNU anonymous structs.
# Tint compiles with -pedantic-errors, turning these into build errors.
if(TARGET tint_lang_msl_validate)
    target_compile_options(tint_lang_msl_validate PRIVATE
        -Wno-gnu-anonymous-struct -Wno-nested-anon-types)
endif()

add_library(mbgl-vendor-dawn INTERFACE)

if(TARGET dawn::webgpu_dawn)
    target_link_libraries(mbgl-vendor-dawn INTERFACE dawn::webgpu_dawn)
else()
    # Xcode path: link individual Dawn component targets.
    # Use BUILD_INTERFACE to avoid export-set requirements.
    target_link_libraries(mbgl-vendor-dawn INTERFACE
        $<BUILD_INTERFACE:dawn_native>
        $<BUILD_INTERFACE:dawn_proc>
    )
endif()

target_include_directories(mbgl-vendor-dawn
    SYSTEM INTERFACE
        ${DAWN_DIR}/include
        ${DAWN_BUILD_DIR}/gen/include
)

if(APPLE)
    target_compile_definitions(mbgl-vendor-dawn INTERFACE DAWN_ENABLE_BACKEND_METAL=1)
else()
    target_compile_definitions(mbgl-vendor-dawn INTERFACE DAWN_ENABLE_BACKEND_VULKAN=1)
endif()

target_compile_options(mbgl-vendor-dawn INTERFACE -Wno-strict-aliasing -Wno-error=strict-aliasing)

set_target_properties(
    mbgl-vendor-dawn
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "dawn"
        INTERFACE_MAPLIBRE_URL "https://dawn.googlesource.com/dawn"
        INTERFACE_MAPLIBRE_AUTHOR "Chromium Dawn Team"
        INTERFACE_MAPLIBRE_LICENSE "${PROJECT_SOURCE_DIR}/vendor/dawn/LICENSE"
)
