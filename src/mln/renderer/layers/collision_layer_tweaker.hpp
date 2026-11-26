#pragma once

#include <mln/renderer/layer_tweaker.hpp>

#include <string>

namespace mln {

/**
    Collision drawables' layer tweaker
 */
class CollisionLayerTweaker : public LayerTweaker {
public:
    CollisionLayerTweaker(std::string name, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(name), properties) {}

public:
    ~CollisionLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;
};

} // namespace mln
