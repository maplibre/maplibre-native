#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/gfx/uniform_buffer.hpp>

#include <string>

namespace mbgl {

/**
    Location indicator layer specific tweaker
 */
class LocationIndicatorLayerTweaker : public LayerTweaker {
public:
    LocationIndicatorLayerTweaker(std::string id_,
                                  Immutable<style::LayerProperties> properties,
                                  const mbgl::mat4& projectionCircle_,
                                  const mbgl::mat4& projectionPuck_)
        : LayerTweaker(std::move(id_), properties),
          projectionCircle(projectionCircle_),
          projectionPuck(projectionPuck_) {}

public:
    ~LocationIndicatorLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters& params) override;

private:
    const mbgl::mat4& projectionCircle;
    const mbgl::mat4& projectionPuck;
};

} // namespace mbgl
