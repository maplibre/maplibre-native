#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string_view>

namespace mbgl {

namespace gfx {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

/**
    Line layer specific tweaker
 */
class LineLayerTweaker : public LayerTweaker {
public:
    LineLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~LineLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;
};

/// Evaluated properties that depend on the tile
struct alignas(16) LinePatternTilePropertiesUBO {
    std::array<float, 4> pattern_from;
    std::array<float, 4> pattern_to;
};
static_assert(sizeof(LinePatternTilePropertiesUBO) %16 == 0);
static constexpr std::string_view LinePatternTilePropertiesUBOName = "LinePatternTilePropertiesUBO";

} // namespace mbgl
