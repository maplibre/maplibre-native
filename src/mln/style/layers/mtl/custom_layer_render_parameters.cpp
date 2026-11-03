#include <mln/style/layers/mtl/custom_layer_render_parameters.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/layer_group.hpp>
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

        // RenderTerrain::renderDepth draws the depth target before the main pass whenever the
        // terrain has depth drawables. Before it has drawn at this size there is no Metal texture
        // behind it, and getDepthTexture may still hand back its 1x1 far-plane placeholder.
        if (auto* terrain = paintParameters.terrain) {
            const auto& depthLayerGroup = terrain->getDepthLayerGroup();
            if (depthLayerGroup && !depthLayerGroup->empty()) {
                const auto& depthTexture = terrain->getDepthTexture(paintParameters.context);
                if (depthTexture && depthTexture->getSize() == renderable.getSize()) {
                    terrainDepthTexture = static_cast<mln::mtl::Texture2D&>(*depthTexture).getMetalTexture();
                }
            }
        }
    }
}

} // namespace mtl
} // namespace style
} // namespace mln
