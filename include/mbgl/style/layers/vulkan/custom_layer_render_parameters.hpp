#pragma once

#include <mbgl/style/layers/custom_layer_render_parameters.hpp>
#include <vulkan/vulkan.hpp>

namespace mbgl {

class PaintParameters;

namespace style {

namespace vulkan {

/**
 * Vulkan subclass of CustomLayerRenderParameters
 */
struct CustomLayerRenderParameters : mbgl::style::CustomLayerRenderParameters {
    /// Dynamic dispatcher.
    const vk::detail::DispatchLoaderDynamic& dispatcher;
    /// Vulkan device
    vk::Device device;
    /// Compatible render pass handle — use when creating VkPipeline.
    vk::RenderPass renderPass;
    /// Active command buffer for the current render pass — record all draw calls here.
    vk::CommandBuffer commandBuffer;

    CustomLayerRenderParameters(const PaintParameters&);
};

} // namespace vulkan
} // namespace style
} // namespace mbgl
