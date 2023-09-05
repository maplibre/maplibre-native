#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string>

namespace mbgl {

/**
    Collision drawables' layer tweaker
 */
class CollisionLayerTweaker : public LayerTweaker {
public:
    CollisionLayerTweaker(std::string name, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(name), properties){}

public:
    ~CollisionLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

    static constexpr std::string_view CollisionCircleUBOName = "CollisionCircleUBO";
    static constexpr std::string_view CollisionBoxUBOName = "CollisionBoxUBO";
};

} // namespace mbgl
