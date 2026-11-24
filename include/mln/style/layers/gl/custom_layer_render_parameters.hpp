#pragma once

#include <mln/style/layers/custom_layer_render_parameters.hpp>

#include <cstdint>

namespace mln {

class PaintParameters;

namespace style {
namespace gl {

/**
 * OpenGL subclass of CustomLayerRenderParameters. A custom layer that wants the terrain depth
 * texture casts the parameters it is given to this type, as Metal and Vulkan layers do to theirs.
 */
struct CustomLayerRenderParameters : mln::style::CustomLayerRenderParameters {
    /// Name of the terrain depth pass's packed-RGBA GL_TEXTURE_2D for this frame, or 0 when terrain is
    /// off or the pass has not drawn yet. Pixel size is the drawable's. Each pixel holds the mesh's
    /// window depth in [0, 1] packed as depth = r / 256^3 + g / 256^2 + b / 256 + a (decode with the
    /// engine's `unpack_depth`); uncovered pixels are white. It is sampled with nearest filtering
    /// and clamped edges.
    std::uint32_t terrainDepthTexture = 0;

    CustomLayerRenderParameters(const PaintParameters& paintParameters, std::uint32_t terrainDepthTexture_)
        : mln::style::CustomLayerRenderParameters(paintParameters),
          terrainDepthTexture(terrainDepthTexture_) {}
};

} // namespace gl
} // namespace style
} // namespace mln
