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
    CollisionLayerTweaker(std::string name, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(name), properties) {}

public:
    ~CollisionLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

    static constexpr auto CollisionCircleUBOName = "CollisionCircleUBO";
    static const size_t idCollisionCircleUBOName;
    static constexpr auto CollisionBoxUBOName = "CollisionBoxUBO";
    static const size_t idCollisionBoxUBOName;
};

} // namespace mbgl
