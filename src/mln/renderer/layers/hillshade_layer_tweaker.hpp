#pragma once

#include <mln/renderer/layer_tweaker.hpp>

#include <array>

namespace mln {

class UnwrappedTileID;
class WorldCoordinateHelper;

/// The tile's latitudes, top and bottom, by which the shader divides slopes; a world that does not wrap has none.
std::array<float, 2> hillshadeLatRange(const UnwrappedTileID&, const WorldCoordinateHelper&);

/**
    Hillshade layer specific tweaker
 */
class HillshadeLayerTweaker : public LayerTweaker {
public:
    HillshadeLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~HillshadeLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
    gfx::UniformBufferPtr tilePropsUniformBuffer;
#endif
};

} // namespace mln
