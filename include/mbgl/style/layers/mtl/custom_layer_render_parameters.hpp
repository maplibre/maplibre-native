#pragma once

#include <mbgl/style/layers/custom_layer_render_parameters.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#include <memory>

namespace mbgl {

class PaintParameters;

namespace style {

namespace mtl {

/**
 * Metal subclass of CustomLayerRenderParameters
 */
struct CustomLayerRenderParameters : mbgl::style::CustomLayerRenderParameters {
    const std::unique_ptr<mbgl::gfx::RenderPass> &renderPass;
    mbgl::mtl::MTLRenderCommandEncoderPtr encoder;
    mbgl::mtl::MTLCommandBufferPtr commandBuffer;
    mbgl::mtl::MTLRenderPassDescriptorPtr renderPassDesc;

    CustomLayerRenderParameters(const PaintParameters&);
};

} // namespace mtl
} // namespace style
} // namespace mbgl
