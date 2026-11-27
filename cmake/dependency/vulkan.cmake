# The Vulkan headers and the Vulkan Memory Allocator, as mbgl-vulkan-headers and
# mbgl-vulkan-memory-allocator: installed ones if there are both, at least as new
# as the vendored copies, unless MLN_USE_BUILTIN_VULKAN is set, and the vendored
# copies otherwise. They go together, as one vendor file defines both.

if(TARGET mbgl-vulkan-headers)
    return()
endif()

if(NOT MLN_USE_BUILTIN_VULKAN)
    find_package(VulkanHeaders CONFIG QUIET)
    find_package(VulkanMemoryAllocator CONFIG QUIET)
endif()

if(VulkanHeaders_FOUND
   AND VulkanHeaders_VERSION VERSION_GREATER_EQUAL 1.4.352
   AND VulkanMemoryAllocator_FOUND
   AND VulkanMemoryAllocator_VERSION VERSION_GREATER_EQUAL 3.1.0)
    message(STATUS "Using installed Vulkan headers ${VulkanHeaders_VERSION} and Vulkan Memory Allocator ${VulkanMemoryAllocator_VERSION}.")
    add_library(mbgl-vulkan-headers ALIAS Vulkan::Headers)
    add_library(mbgl-vulkan-memory-allocator ALIAS GPUOpen::VulkanMemoryAllocator)
else()
    set(MLN_USE_BUILTIN_VULKAN TRUE)
    include(${PROJECT_SOURCE_DIR}/vendor/vulkan.cmake)
    add_library(mbgl-vulkan-headers ALIAS mbgl-vendor-vulkan-headers)
    add_library(mbgl-vulkan-memory-allocator ALIAS mbgl-vendor-VulkanMemoryAllocator)
endif()
