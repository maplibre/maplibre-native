add_library(mbgl-vendor-vulkan-headers INTERFACE)

target_include_directories(
    mbgl-vendor-vulkan-headers SYSTEM
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/Vulkan-Headers/include/
)

set_target_properties(
    mbgl-vendor-vulkan-headers
    PROPERTIES
        INTERFACE_MAPBOX_NAME "Vulkan-Headers"
        INTERFACE_MAPBOX_URL "https://github.com/KhronosGroup/Vulkan-Headers.git"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/Vulkan-Headers/LICENSE.md
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
        INTERFACE_MAPBOX_NAME "VulkanMemoryAllocator"
        INTERFACE_MAPBOX_URL "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/VulkanMemoryAllocator/LICENSE.txt
)

set(ENABLE_OPT OFF)
set(ENABLE_HLSL OFF)
set(ENABLE_GLSLANG_BINARIES OFF)
set(GLSLANG_TESTS_DEFAULT OFF)
set(ENABLE_PCH OFF)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/glslang SYSTEM)

get_target_property(glslang_inc glslang INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(glslang PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${glslang_inc})

set_target_properties(
    glslang
    PROPERTIES
        INTERFACE_MAPBOX_NAME "glslang"
        INTERFACE_MAPBOX_URL "https://github.com/KhronosGroup/glslang.git"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/glslang/LICENSE.txt
)

set_target_properties(
    SPIRV
    PROPERTIES
        INTERFACE_MAPBOX_NAME "SPIRV"
        INTERFACE_MAPBOX_URL "https://github.com/KhronosGroup/glslang.git"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/glslang/LICENSE.txt
)

set_target_properties(
    glslang-default-resource-limits
    PROPERTIES
        INTERFACE_MAPBOX_NAME "glslang-default-resource-limits"
        INTERFACE_MAPBOX_URL "https://github.com/KhronosGroup/glslang.git"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/glslang/LICENSE.txt
)

export(TARGETS
    mbgl-vendor-vulkan-headers
    mbgl-vendor-VulkanMemoryAllocator
    glslang
    SPIRV
    glslang-default-resource-limits
    MachineIndependent
    OSDependent
    GenericCodeGen
    
    APPEND FILE MapboxCoreTargets.cmake
)