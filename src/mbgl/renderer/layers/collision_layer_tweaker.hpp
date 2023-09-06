#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/util/string_indexer.hpp>

#include <string>

namespace mbgl {

/**
    Collision drawables' layer tweaker
 */
class CollisionLayerTweaker : public LayerTweaker {
public:
    CollisionLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~CollisionLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

    static constexpr auto CollisionCircleUBOName = "CollisionCircleUBO";
    static const StringIdentity idCollisionCircleUBOName;
    static constexpr auto CollisionBoxUBOName = "CollisionBoxUBO";
    static const StringIdentity idCollisionBoxUBOName;
};

} // namespace mbgl
