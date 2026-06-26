#pragma once

#include <mbgl/style/layers/custom_layer_init_parameters.hpp>

#include <vulkan/vulkan.hpp>

namespace mbgl {
namespace style {
namespace vulkan {

/**
 * Vulkan-specific initialization parameters.
 * Provides device handles so that custom layers can create pipelines,
 * allocate buffers, and compile shaders during initialize().
 *
 * Note: vk::RenderPass is NOT available here because it depends on the
 * active framebuffer which may not exist yet. Use the render pass from
 * CustomLayerRenderParameters during the first render() call if needed
 * for pipeline creation.
 */
struct CustomLayerInitParameters : mbgl::style::CustomLayerInitParameters {
    /// Dynamic dispatcher for Vulkan function calls.
    const vk::detail::DispatchLoaderDynamic& dispatcher;
    /// Logical device handle.
    vk::Device device;
    /// Physical device handle — useful for querying memory properties, limits, etc.
    vk::PhysicalDevice physicalDevice;

    CustomLayerInitParameters(const vk::detail::DispatchLoaderDynamic& dispatcher_,
                              vk::Device device_,
                              vk::PhysicalDevice physicalDevice_)
        : dispatcher(dispatcher_),
          device(device_),
          physicalDevice(physicalDevice_) {}
};

} // namespace vulkan
} // namespace style
} // namespace mbgl
