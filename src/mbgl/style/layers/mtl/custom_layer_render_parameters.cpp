#include <mbgl/style/layers/mtl/custom_layer_render_parameters.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/mtl/render_pass.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

namespace mbgl {
namespace style {
namespace mtl {

CustomLayerRenderParameters::CustomLayerRenderParameters(const mbgl::PaintParameters& paintParameters)
    : mbgl::style::CustomLayerRenderParameters(paintParameters) {
    const mbgl::mtl::RenderPass& renderPass = static_cast<mbgl::mtl::RenderPass&>(*paintParameters.renderPass);
    encoder = renderPass.getMetalEncoder();
}

} // namespace mtl
} // namespace style
} // namespace mbgl
