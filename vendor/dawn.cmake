# Dawn WebGPU Implementation Configuration

if(MLN_WITH_WEBGPU)
    message(STATUS "Configuring Dawn WebGPU implementation")

    # Dawn library paths (bundled in the repository)
    set(DAWN_DIR "${PROJECT_SOURCE_DIR}/vendor/dawn")
    set(DAWN_BUILD_DIR "${DAWN_DIR}/build")

    # Add Dawn include directories
    target_include_directories(mbgl-core
        PUBLIC
            ${DAWN_DIR}/include
            ${DAWN_BUILD_DIR}/gen/include
            ${DAWN_DIR}/src  # Add src directory for internal headers
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

    target_link_libraries(mbgl-core PUBLIC ${DAWN_CORE_LIBS})

    # Link Tint libraries (shader compiler)
    file(GLOB TINT_LIBS "${DAWN_BUILD_DIR}/src/tint/*.a")
    target_link_libraries(mbgl-core PUBLIC ${TINT_LIBS})

    # Link abseil libraries (Dawn dependency)
    file(GLOB_RECURSE ABSEIL_LIBS "${DAWN_BUILD_DIR}/third_party/abseil/*.a")
    target_link_libraries(mbgl-core PUBLIC ${ABSEIL_LIBS})

    # Link other third party libraries
    file(GLOB THIRD_PARTY_LIBS
        "${DAWN_BUILD_DIR}/third_party/protobuf/*.a"
        "${DAWN_BUILD_DIR}/third_party/glslang/**/*.a"
    )
    target_link_libraries(mbgl-core PUBLIC ${THIRD_PARTY_LIBS})

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
    else()
        target_link_libraries(mbgl-core PUBLIC dl pthread)
    endif()

    # Define that we're using Dawn
    target_compile_definitions(mbgl-core PUBLIC MLN_WITH_DAWN=1 WEBGPU_BACKEND_DAWN=1)

    if(APPLE)
        target_compile_definitions(mbgl-core PUBLIC DAWN_ENABLE_BACKEND_METAL=1)
    else()
        target_compile_definitions(mbgl-core PUBLIC DAWN_ENABLE_BACKEND_VULKAN=1)
    endif()
endif()
