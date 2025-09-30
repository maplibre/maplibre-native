# Dawn WebGPU Implementation Configuration

if(TARGET mbgl-vendor-dawn)
    return()
endif()

if(NOT MLN_WITH_WEBGPU)
    return()
endif()

message(STATUS "Configuring Dawn WebGPU implementation")

# Dawn library paths (bundled in the repository)
set(DAWN_DIR "${PROJECT_SOURCE_DIR}/vendor/dawn")
set(DAWN_BUILD_DIR "${DAWN_DIR}/build")

add_library(mbgl-vendor-dawn INTERFACE)

# Publish Dawn include directories to consumers
target_include_directories(mbgl-vendor-dawn
    INTERFACE
        ${DAWN_DIR}/include
        ${DAWN_BUILD_DIR}/gen/include
        ${DAWN_DIR}/src  # Internal headers needed by bundled build
)

# Link core Dawn libraries (order matters for static linking)
set(DAWN_CORE_LIBS
    ${DAWN_BUILD_DIR}/src/dawn/native/libwebgpu_dawn.a
    ${DAWN_BUILD_DIR}/src/dawn/libdawn_proc.a
    ${DAWN_BUILD_DIR}/src/dawn/platform/libdawn_platform.a
    ${DAWN_BUILD_DIR}/src/dawn/common/libdawn_common.a
)

if(EXISTS "${DAWN_BUILD_DIR}/src/dawn/native/libdawn_native.a")
    list(APPEND DAWN_CORE_LIBS ${DAWN_BUILD_DIR}/src/dawn/native/libdawn_native.a)
endif()

if(EXISTS "${DAWN_BUILD_DIR}/src/dawn/wire/libdawn_wire.a")
    list(APPEND DAWN_CORE_LIBS ${DAWN_BUILD_DIR}/src/dawn/wire/libdawn_wire.a)
endif()

if(DAWN_CORE_LIBS)
    target_link_libraries(mbgl-vendor-dawn INTERFACE ${DAWN_CORE_LIBS})
endif()

# Link Tint libraries (shader compiler)
file(GLOB TINT_LIBS "${DAWN_BUILD_DIR}/src/tint/*.a")
if(TINT_LIBS)
    target_link_libraries(mbgl-vendor-dawn INTERFACE ${TINT_LIBS})
endif()

# Link abseil libraries (Dawn dependency)
file(GLOB_RECURSE ABSEIL_LIBS "${DAWN_BUILD_DIR}/third_party/abseil/*.a")
if(ABSEIL_LIBS)
    target_link_libraries(mbgl-vendor-dawn INTERFACE ${ABSEIL_LIBS})
endif()

# Link other third party libraries
file(GLOB THIRD_PARTY_LIBS
    "${DAWN_BUILD_DIR}/third_party/protobuf/*.a"
    "${DAWN_BUILD_DIR}/third_party/glslang/**/*.a"
)
if(THIRD_PARTY_LIBS)
    target_link_libraries(mbgl-vendor-dawn INTERFACE ${THIRD_PARTY_LIBS})
endif()

# Platform-specific libraries
if(NOT APPLE)
    target_link_libraries(mbgl-vendor-dawn INTERFACE dl pthread)
endif()

# Define that we're using Dawn
target_compile_definitions(mbgl-vendor-dawn
    INTERFACE
        MLN_WITH_DAWN=1
        WEBGPU_BACKEND_DAWN=1
)

if(APPLE)
    target_compile_definitions(mbgl-vendor-dawn INTERFACE DAWN_ENABLE_BACKEND_METAL=1)
else()
    target_compile_definitions(mbgl-vendor-dawn INTERFACE DAWN_ENABLE_BACKEND_VULKAN=1)
endif()

# Dawn's generated C++ headers rely on reinterpret casts that trigger
# -Wstrict-aliasing diagnostics under GCC; silence these when the Dawn
# backend is enabled so the build doesn't fail with -Werror.
target_compile_options(mbgl-vendor-dawn INTERFACE -Wno-strict-aliasing -Wno-error=strict-aliasing)

set_target_properties(
    mbgl-vendor-dawn
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "dawn"
        INTERFACE_MAPLIBRE_URL "https://dawn.googlesource.com/dawn"
        INTERFACE_MAPLIBRE_AUTHOR "Chromium Dawn Team"
        INTERFACE_MAPLIBRE_LICENSE "${PROJECT_SOURCE_DIR}/vendor/dawn/LICENSE"
)
