set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

# Force Visual Studio generator instead of Ninja for ARM64
set(VCPKG_CMAKE_GENERATOR "Visual Studio 17 2022")
set(VCPKG_CMAKE_GENERATOR_PLATFORM "ARM64")

# Explicitly set the platform toolset
set(VCPKG_PLATFORM_TOOLSET v143)