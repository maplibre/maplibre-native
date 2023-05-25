#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/layout/pattern_layout.hpp>

#include <memory>

namespace mbgl {

class FillBucket;
class FillProgram;
class FillPatternProgram;
class FillOutlineProgram;
class FillOutlinePatternProgram;
class TileLayerGroup;
using TileLayerGroupPtr = std::shared_ptr<TileLayerGroup>;

class RenderFillLayer final : public RenderLayer {
public:
    explicit RenderFillLayer(Immutable<style::FillLayer::Impl>);
    ~RenderFillLayer() override;

    void layerRemoved(UniqueChangeRequestVec&) override;

    /// Generate any changes needed by the layer
    void update(int32_t layerIndex,
                gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                UniqueChangeRequestVec&) override;

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void render(PaintParameters&) override;

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;

    /// Remove all drawables for the tile from the layer group
    void removeTile(RenderPass, const OverscaledTileID&);

    // Paint properties
    style::FillPaintProperties::Unevaluated unevaluated;

    // Programs
    std::shared_ptr<FillProgram> fillProgram;
    std::shared_ptr<FillPatternProgram> fillPatternProgram;
    std::shared_ptr<FillOutlineProgram> fillOutlineProgram;
    std::shared_ptr<FillOutlinePatternProgram> fillOutlinePatternProgram;

    gfx::ShaderProgramBasePtr fillShader;
    gfx::ShaderProgramBasePtr outlineShader;
    gfx::ShaderProgramBasePtr patternShader;
    gfx::ShaderProgramBasePtr outlinePatternShader;
};

struct alignas(16) FillDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 4> scale;
    /*  80 */ std::array<float, 2> world;
    /*  88 */ std::array<float, 2> pixel_coord_upper;
    /*  96 */ std::array<float, 2> pixel_coord_lower;
    /* 104 */ std::array<float, 2> texsize;
    /* 112 */ float fade;

    // Attribute interpolations (used without HAS_UNIFORM_u_*)
    /* 116 */ float color_t;
    /* 120 */ float opacity_t;
    /* 124 */ float outline_color_t;
    /* 128 */ float pattern_from_t;
    /* 132 */ float pattern_to_t;

    // Uniform alternates for attributes (used with HAS_UNIFORM_u_*)
    /* 136 */ std::array<float, 2> color;
    /* 144 */ std::array<float, 2> opacity;
    /* 152 */ std::array<float, 2> outline_color_pad;
    /* 160 */ std::array<float, 4> outline_color;
    /* 176 */ std::array<float, 4> pattern_from;
    /* 208 */ std::array<float, 4> pattern_to;

    // Pattern texture
    /* ? */ // Drawable::TextureAttachment? image;

    /*  */ //std::array<float, 3> padding;
    /* 208 */
};
static_assert(sizeof(FillDrawableUBO) == 208);

} // namespace mbgl
