#pragma once

#include <mln/renderer/layer_tweaker.hpp>

#include <string>

namespace mln {

/// Tweaker for the fill-extrusion ground-shadow drawables.
///
/// Runs against the shadow's own tile layer group (registered ahead of the building layer group at
/// the same layer index, so shadows draw underneath the buildings), and owns the drawable-to-drawable
/// matrix/shear as well as the layer-wide colour/opacity, base and height.
class FillExtrusionShadowLayerTweaker : public LayerTweaker {
public:
    FillExtrusionShadowLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

    ~FillExtrusionShadowLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif
};

} // namespace mln
