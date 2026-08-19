#include <mbgl/style/layers/mtl/custom_layer_render_parameters.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/mtl/render_pass.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

namespace mln {
namespace style {
namespace mtl {

CustomLayerRenderParameters::CustomLayerRenderParameters(const mln::PaintParameters& paintParameters)
    : mln::style::CustomLayerRenderParameters(paintParameters) {
    const mln::mtl::RenderPass& renderPass = static_cast<mln::mtl::RenderPass&>(*paintParameters.renderPass);
    encoder = renderPass.getMetalEncoder();
}

} // namespace mtl
} // namespace style
} // namespace mln
