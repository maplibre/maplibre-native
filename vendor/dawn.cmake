# Dawn WebGPU Implementation Configuration

if(MLN_WITH_WEBGPU)
    message(STATUS "Configuring Dawn WebGPU implementation")

    # Dawn library paths
    set(DAWN_DIR "/Users/admin/repos/dawn")
    set(DAWN_BUILD_DIR "${DAWN_DIR}/build")

    # Add Dawn include directories
    target_include_directories(mbgl-core
        PUBLIC
            ${DAWN_DIR}/include
            ${DAWN_BUILD_DIR}/gen/include
    )

    # Find all Dawn libraries we need
    find_library(DAWN_NATIVE_LIB dawn_native PATHS ${DAWN_BUILD_DIR}/src/dawn/native NO_DEFAULT_PATH)
    find_library(DAWN_PROC_LIB dawn_proc PATHS ${DAWN_BUILD_DIR}/src/dawn NO_DEFAULT_PATH)
    find_library(DAWN_PLATFORM_LIB dawn_platform PATHS ${DAWN_BUILD_DIR}/src/dawn/platform NO_DEFAULT_PATH)
    find_library(DAWN_COMMON_LIB dawn_common PATHS ${DAWN_BUILD_DIR}/src/dawn/common NO_DEFAULT_PATH)
    find_library(DAWN_WIRE_LIB dawn_wire PATHS ${DAWN_BUILD_DIR}/src/dawn/wire NO_DEFAULT_PATH)

    # Link Dawn libraries
    target_link_libraries(mbgl-core
        PUBLIC
            ${DAWN_BUILD_DIR}/src/dawn/native/libdawn_native.a
            ${DAWN_BUILD_DIR}/src/dawn/libdawn_proc.a
            ${DAWN_BUILD_DIR}/src/dawn/platform/libdawn_platform.a
            ${DAWN_BUILD_DIR}/src/dawn/common/libdawn_common.a
            ${DAWN_BUILD_DIR}/src/dawn/wire/libdawn_wire.a
    )

    # Link Tint libraries
    file(GLOB TINT_LIBS "${DAWN_BUILD_DIR}/src/tint/*.a")
    target_link_libraries(mbgl-core PUBLIC ${TINT_LIBS})

    # Link abseil libraries
    file(GLOB_RECURSE ABSEIL_LIBS "${DAWN_BUILD_DIR}/third_party/abseil/*.a")
    target_link_libraries(mbgl-core PUBLIC ${ABSEIL_LIBS})

    # Platform-specific libraries
    if(APPLE)
        target_link_libraries(mbgl-core
            PUBLIC
                "-framework Metal"
                "-framework QuartzCore"
                "-framework IOKit"
                "-framework IOSurface"
                "-framework CoreGraphics"
        )
    endif()

    # Define that we're using Dawn
    target_compile_definitions(mbgl-core
        PUBLIC
            DAWN_ENABLE_BACKEND_METAL=1
            USE_DAWN=1
    )
endif()
