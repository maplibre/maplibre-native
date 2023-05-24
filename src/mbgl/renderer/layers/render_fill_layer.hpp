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

struct alignas(16) FillLayerUBO {
    /*   0 */ std::array<float, 4> scale;
    /*  16 */ std::array<float, 2> pixel_coord_upper;
    /*  24 */ std::array<float, 2> pixel_coord_lower;
    /*  32 */ std::array<float, 2> texsize;
    /*  40 */ float fade;

    // Attribute interpolations
    /*  44 */ float color_t;
    /*  48 */ float opacity_t;
    /*  52 */ float outline_color_t;
    /*  56 */ float pattern_from_t;
    /*  60 */ float pattern_to_t;

    /*  64 */ std::array<float, 2> u_color;
    /*  72 */ std::array<float, 2> u_opacity;
    /*  80 */ std::array<float, 4> u_outline_color;
    /*  96 */ std::array<float, 4> u_pattern_from;
    /* 112 */ std::array<float, 4> u_pattern_to;

    // Pattern texture
    /* ? */ // Drawable::TextureAttachment? image;

    /*  */ //std::array<float, 3> padding;
    /* 128 */
};
static_assert(sizeof(FillLayerUBO) == 128);

} // namespace mbgl
