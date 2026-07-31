#include <mbgl/style/layers/mtl/custom_layer_render_parameters.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/renderable_resource.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

namespace mbgl {
namespace style {
namespace mtl {

CustomLayerRenderParameters::CustomLayerRenderParameters(const mbgl::PaintParameters& paintParameters)
    : mbgl::style::CustomLayerRenderParameters(paintParameters),
      renderPass(paintParameters.renderPass) {
    const mbgl::gfx::Renderable& renderable = paintParameters.backend.getDefaultRenderable();
    const mbgl::mtl::RenderableResource& resource = renderable.getResource<mbgl::mtl::RenderableResource>();
    renderPassDesc = renderable.getResource<mbgl::mtl::RenderableResource>().getRenderPassDescriptor();
    if (const auto& buffer_ = resource.getCommandBuffer()) {
        commandBuffer = buffer_;
    }
    if (paintParameters.renderPass) {
        const mbgl::mtl::RenderPass& renderPass = static_cast<mbgl::mtl::RenderPass&>(*paintParameters.renderPass);
        encoder = renderPass.getMetalEncoder();
    }
}

} // namespace mtl
} // namespace style
} // namespace mbgl
