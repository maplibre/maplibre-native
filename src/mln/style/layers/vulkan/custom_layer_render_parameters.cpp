// Include this before including custom_layer_render_parameters as custom_layer_render_parameters needs to be
// "lightweight". It shouldn't touch the full <mln/vulkan/renderer_backend.hpp> as it brings the whole new big
#include <mln/vulkan/renderer_backend.hpp>
#include <mln/vulkan/context.hpp>
#include <mln/style/layers/vulkan/custom_layer_render_parameters.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/vulkan/command_encoder.hpp>
#include <mln/vulkan/render_pass.hpp>
#include <mln/vulkan/renderable_resource.hpp>

namespace mln {
namespace style {
namespace vulkan {

CustomLayerRenderParameters::CustomLayerRenderParameters(const mln::PaintParameters& paintParameters)
    : mln::style::CustomLayerRenderParameters(paintParameters),
      dispatcher(static_cast<mln::vulkan::RendererBackend&>(paintParameters.backend).getDispatcher()) {
    auto& backend = static_cast<mln::vulkan::RendererBackend&>(paintParameters.backend);
    device = backend.getDevice().get();
    commandBuffer = backend.getContext<mln::vulkan::Context>().getCommandBuffer().get();

    const auto& renderable = paintParameters.backend.getDefaultRenderable();
    const auto& resource = renderable.getResource<mln::vulkan::RenderableResource>();
    renderPass = resource.getRenderPass().get();
    screenPreRotationRadiansClockwise = resource.getRotation();
}

} // namespace vulkan
} // namespace style
} // namespace mln
