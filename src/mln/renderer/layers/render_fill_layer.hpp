#pragma once

#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/fill_layer_impl.hpp>
#include <mln/style/layers/fill_layer_properties.hpp>
#include <mln/layout/pattern_layout.hpp>
#include <mln/renderer/buckets/fill_bucket.hpp>

#include <memory>

namespace mln {

class FillLayerTweaker;
using FillLayerTweakerPtr = std::shared_ptr<FillLayerTweaker>;

class RenderFillLayer final : public RenderLayer {
public:
    enum class FillVariant : uint8_t {
        Fill,
        FillPattern,
        FillOutline,
        FillOutlinePattern,
        FillOutlineTriangulated,
        Undefined = 255
    };

public:
    explicit RenderFillLayer(Immutable<style::FillLayer::Impl>);
    ~RenderFillLayer() override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry &,
                gfx::Context &,
                const TransformState &,
                const std::shared_ptr<UpdateParameters> &,
                const PaintParameters &,
                const RenderTree &,
                UniqueChangeRequestVec &) override;

private:
    void transition(const TransitionParameters &) override;
    void evaluate(const PropertyEvaluationParameters &) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;

    bool queryIntersectsFeature(const GeometryCoordinates &,
                                const GeometryTileFeature &,
                                float,
                                const TransformState &,
                                float,
                                const mat4 &,
                                const FeatureState &) const override;

    // Paint properties
    style::FillPaintProperties::Unevaluated unevaluated;

    gfx::ShaderGroupPtr fillShaderGroup;
    gfx::ShaderGroupPtr outlineShaderGroup;
    gfx::ShaderGroupPtr patternShaderGroup;
    gfx::ShaderGroupPtr outlinePatternShaderGroup;

#if MLN_TRIANGULATE_FILL_OUTLINES
    gfx::ShaderGroupPtr outlineTriangulatedShaderGroup;
#endif // MLN_TRIANGULATE_FILL_OUTLINES
};

} // namespace mln
