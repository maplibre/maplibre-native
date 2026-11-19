#pragma once

#include <mln/renderer/layer_tweaker.hpp>

#include <string>

namespace mln {

/// Tweaker for the fill-extrusion ground ambient occlusion drawables.
///
/// Runs against the occlusion's own tile layer group, registered after the building layer group at
/// the same layer index, and sets the tile matrix, the metres-to-tile-units scale and the
/// layer-wide intensity, radius, base and height.
class FillExtrusionGroundOcclusionLayerTweaker : public LayerTweaker {
public:
    FillExtrusionGroundOcclusionLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

    ~FillExtrusionGroundOcclusionLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif
};

} // namespace mln
