# Minimal ARM64 Windows toolchain for MapLibre Native

# Set vcpkg for ARM64
set(VCPKG_TARGET_TRIPLET "arm64-windows" CACHE STRING "")
set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg-custom-triplets)

# Include the base vcpkg toolchain first
include(${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake)

# Ensure ARM64 packages are found by adding to CMAKE_PREFIX_PATH after vcpkg toolchain
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/arm64-windows")