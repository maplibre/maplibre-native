#pragma once

#include <mln/renderer/layer_tweaker.hpp>

namespace mln {

class WorldCoordinateHelper;

/// The log2 of the zoom 0 world's width in meters, from which the prepare shader gets the meters per DEM pixel. A
/// registered projection's world is its zoom 0 extent wide; Mercator keeps 25.2562, which with the shader's 3 is the
/// 28.2562 it always used (GL JS `getLog2WorldWidth`).
float hillshadeLog2WorldWidth(const WorldCoordinateHelper&);

/**
    Hillshade prepare layer specific tweaker
 */
class HillshadePrepareLayerTweaker : public LayerTweaker {
public:
    HillshadePrepareLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~HillshadePrepareLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
};

} // namespace mln
