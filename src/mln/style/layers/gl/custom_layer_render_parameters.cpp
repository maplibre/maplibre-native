#include <mln/style/layers/gl/custom_layer_render_parameters.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/gl/texture2d.hpp>

namespace mln {
namespace style {
namespace gl {

CustomLayerRenderParameters::CustomLayerRenderParameters(const mln::PaintParameters& paintParameters)
    : mln::style::CustomLayerRenderParameters(paintParameters) {
    if (auto* terrain = paintParameters.terrain) {
        if (auto* depthTexture = terrain->getDrawnDepthTexture(
                paintParameters.context, paintParameters.backend.getDefaultRenderable().getSize())) {
            terrainDepthTexture = static_cast<mln::gl::Texture2D&>(*depthTexture).getTextureID();
        }
    }
}

} // namespace gl
} // namespace style
} // namespace mln
