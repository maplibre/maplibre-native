# ARM64 Windows toolchain for MapLibre Native

# Set vcpkg for ARM64
set(VCPKG_TARGET_TRIPLET "arm64-windows" CACHE STRING "")
set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg-custom-triplets)

# Add path to vcpkg ARM64 dependencies
list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/installed/arm64-windows")

# Include the base vcpkg toolchain
include(${CMAKE_CURRENT_LIST_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake)