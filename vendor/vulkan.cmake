add_library(mbgl-vendor-vulkan-headers INTERFACE)

target_include_directories(
    mbgl-vendor-vulkan-headers SYSTEM
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/Vulkan-Headers/include/
)

set_target_properties(
    mbgl-vendor-vulkan-headers
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "Vulkan-Headers"
        INTERFACE_MAPLIBRE_URL "https://github.com/KhronosGroup/Vulkan-Headers.git"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/Vulkan-Headers/LICENSE.md
)

add_library(mbgl-vendor-VulkanMemoryAllocator INTERFACE)

target_include_directories(
    mbgl-vendor-VulkanMemoryAllocator SYSTEM
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/VulkanMemoryAllocator/include/
)

set_target_properties(
    mbgl-vendor-VulkanMemoryAllocator
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "VulkanMemoryAllocator"
        INTERFACE_MAPLIBRE_URL "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/VulkanMemoryAllocator/LICENSE.txt
)

export(TARGETS
    mbgl-vendor-vulkan-headers
    mbgl-vendor-VulkanMemoryAllocator

    APPEND FILE MapboxCoreTargets.cmake
)
