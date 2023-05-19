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

    // Paint properties
    style::FillPaintProperties::Unevaluated unevaluated;

    // Programs
    std::shared_ptr<FillProgram> fillProgram;
    std::shared_ptr<FillPatternProgram> fillPatternProgram;
    std::shared_ptr<FillOutlineProgram> fillOutlineProgram;
    std::shared_ptr<FillOutlinePatternProgram> fillOutlinePatternProgram;

    gfx::ShaderProgramBasePtr fillShader;
    gfx::ShaderProgramBasePtr outlineShader;
};

} // namespace mbgl
