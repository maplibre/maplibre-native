#pragma once

#include <mln/renderer/layer_tweaker.hpp>
#include <mln/gfx/uniform_buffer.hpp>

#include <string>

namespace mln {

/**
    Location indicator layer specific tweaker
 */
class LocationIndicatorLayerTweaker : public LayerTweaker {
public:
    LocationIndicatorLayerTweaker(std::string id_,
                                  Immutable<style::LayerProperties> properties,
                                  const mln::mat4& projectionCircle_,
                                  const mln::mat4& projectionPuck_)
        : LayerTweaker(std::move(id_), properties),
          projectionCircle(projectionCircle_),
          projectionPuck(projectionPuck_) {}

public:
    ~LocationIndicatorLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters& params) override;

private:
    const mln::mat4& projectionCircle;
    const mln::mat4& projectionPuck;
};

} // namespace mln
