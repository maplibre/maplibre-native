// Include this before including custom_layer_render_parameters as custom_layer_render_parameters needs to be
// "lightweight". It shouldn't touch the full <mbgl/vulkan/renderer_backend.hpp> as it brings the whole new big
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/style/layers/vulkan/custom_layer_render_parameters.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>

namespace mbgl {
namespace style {
namespace vulkan {

CustomLayerRenderParameters::CustomLayerRenderParameters(const mbgl::PaintParameters& paintParameters)
    : mbgl::style::CustomLayerRenderParameters(paintParameters),
      dispatcher(static_cast<mbgl::vulkan::RendererBackend&>(paintParameters.backend).getDispatcher()) {
    device = static_cast<mbgl::vulkan::RendererBackend&>(paintParameters.backend).getDevice().get();
    auto& vulkanRenderPass = static_cast<mbgl::vulkan::RenderPass&>(*paintParameters.renderPass);
    commandBuffer = vulkanRenderPass.getEncoder().getCommandBuffer().get();

    const auto& renderable = paintParameters.backend.getDefaultRenderable();
    const auto& resource = renderable.getResource<mbgl::vulkan::RenderableResource>();
    renderPass = resource.getRenderPass().get();
    screenPreRotationRadiansClockwise = resource.getRotation();
}

} // namespace vulkan
} // namespace style
} // namespace mbgl
