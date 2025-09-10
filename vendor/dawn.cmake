# Dawn WebGPU Implementation Configuration

if(MLN_WITH_WEBGPU)
    message(STATUS "Configuring Dawn WebGPU implementation")

    # Dawn library paths
    set(DAWN_DIR "/Users/admin/repos/dawn")
    set(DAWN_BUILD_DIR "${DAWN_DIR}/out/Release")

    # Add Dawn include directories
    target_include_directories(mbgl-core
        PUBLIC
            ${DAWN_DIR}/include
            ${DAWN_DIR}/out/Release/gen/include
    )

    # Link Dawn libraries
    target_link_libraries(mbgl-core
        PUBLIC
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
    )

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
