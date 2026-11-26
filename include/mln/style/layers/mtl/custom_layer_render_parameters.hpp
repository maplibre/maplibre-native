#pragma once

#include <mln/style/layers/custom_layer_render_parameters.hpp>
#include <mln/gfx/render_pass.hpp>
#include <mln/mtl/mtl_fwd.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#include <memory>

namespace mln {

class PaintParameters;

namespace style {

namespace mtl {

/**
 * Metal subclass of CustomLayerRenderParameters
 */
struct CustomLayerRenderParameters : mln::style::CustomLayerRenderParameters {
    const std::unique_ptr<mln::gfx::RenderPass> &renderPass;
    mln::mtl::MTLRenderCommandEncoderPtr encoder;
    mln::mtl::MTLCommandBufferPtr commandBuffer;
    mln::mtl::MTLRenderPassDescriptorPtr renderPassDesc;

    CustomLayerRenderParameters(const PaintParameters &);
};

} // namespace mtl
} // namespace style
} // namespace mln
