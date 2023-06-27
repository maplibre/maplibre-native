#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string>

namespace mbgl {

/**
    Fill extrusion layer specific tweaker
 */
class FillExtrusionLayerTweaker : public LayerTweaker {
public:
    FillExtrusionLayerTweaker(Immutable<style::LayerProperties> properties)
        : LayerTweaker(properties){};

public:
    ~FillExtrusionLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

    static constexpr std::string_view FillExtrusionTilePropsUBOName = "FillExtrusionDrawableTilePropsUBO";
    static constexpr std::string_view FillExtrusionInterpolateUBOName = "FillExtrusionInterpolateUBO";

private:
    gfx::UniformBufferPtr propsBuffer;
};

/// Evaluated properties that depend on the tile
struct alignas(16) FillExtrusionDrawableTilePropsUBO {
    /*  0 */ std::array<float, 4> pattern_from;
    /* 16 */ std::array<float, 4> pattern_to;
    /* 32 */
};
static_assert(sizeof(FillExtrusionDrawableTilePropsUBO) == 32);

/// Attribute interpolations
struct alignas(16) FillExtrusionInterpolateUBO {
    /*  0 */ float color_t;
    /*  4 */ float opacity_t;
    /*  8 */ float outline_color_t;
    /* 12 */ float pattern_from_t;
    /* 16 */ float pattern_to_t;
    /* 20 */ float fade;
    /* 24 */ std::array<float, 2> padding;
    /* 32 */
};
static_assert(sizeof(FillExtrusionInterpolateUBO) == 32);

} // namespace mbgl
