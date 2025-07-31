# ARM64 Windows configuration for MapLibre Native

# Set vcpkg for ARM64
set(VCPKG_TARGET_TRIPLET "arm64-windows" CACHE STRING "")
set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_LIST_DIR}/platform/windows/vendor/vcpkg-custom-triplets)

# Include vcpkg toolchain
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/platform/windows/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Add path to vcpkg ARM64 dependencies
list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/platform/windows/vendor/vcpkg/installed/arm64-windows")