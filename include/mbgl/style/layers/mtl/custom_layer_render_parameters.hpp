#pragma once

#include <mbgl/style/layers/custom_layer_render_parameters.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

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
    mln::mtl::MTLRenderCommandEncoderPtr encoder;

    CustomLayerRenderParameters(const PaintParameters&);
};

} // namespace mtl
} // namespace style
} // namespace mln
