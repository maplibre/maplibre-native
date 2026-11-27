#include <mln/style/layers/mtl/custom_layer_render_parameters.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/mtl/render_pass.hpp>
#include <mln/mtl/renderer_backend.hpp>
#include <mln/mtl/renderable_resource.hpp>
#include <mln/mtl/texture2d.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

namespace mln {
namespace style {
namespace mtl {

CustomLayerRenderParameters::CustomLayerRenderParameters(const mln::PaintParameters& paintParameters)
    : mln::style::CustomLayerRenderParameters(paintParameters),
      renderPass(paintParameters.renderPass) {
    const mln::gfx::Renderable& renderable = paintParameters.backend.getDefaultRenderable();
    const mln::mtl::RenderableResource& resource = renderable.getResource<mln::mtl::RenderableResource>();
    renderPassDesc = renderable.getResource<mln::mtl::RenderableResource>().getRenderPassDescriptor();
    if (const auto& buffer_ = resource.getCommandBuffer()) {
        commandBuffer = buffer_;
    }
    if (paintParameters.renderPass) {
        const mln::mtl::RenderPass& metalRenderPass = static_cast<mln::mtl::RenderPass&>(*paintParameters.renderPass);
        encoder = metalRenderPass.getMetalEncoder();

        if (auto* terrain = paintParameters.terrain) {
            if (auto* depthTexture = terrain->getDrawnDepthTexture(paintParameters.context, renderable.getSize())) {
                terrainDepthTexture = static_cast<mln::mtl::Texture2D&>(*depthTexture).getMetalTexture();
            }
        }
    }
}

} // namespace mtl
} // namespace style
} // namespace mln
