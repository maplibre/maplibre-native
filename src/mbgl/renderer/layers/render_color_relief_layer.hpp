#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/style/layers/color_relief_layer_impl.hpp>
#include <mbgl/style/layers/color_relief_layer_properties.hpp>
#include <mbgl/util/image.hpp>
#include <memory>

namespace mbgl {
namespace gfx {
class Texture2D;
class ShaderProgramBase;
} // namespace gfx
} // namespace mbgl

namespace mbgl {

class RenderColorReliefLayer final : public RenderLayer {
public:
    explicit RenderColorReliefLayer(Immutable<style::ColorReliefLayer::Impl>);
    ~RenderColorReliefLayer() override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    void prepare(const LayerPrepareParameters&) override;
    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;
    void updateColorRamp();

    // Paint properties
    style::ColorReliefPaintProperties::Unevaluated unevaluated;

    // Color ramp data
    uint32_t colorRampSize = 256;
    bool colorRampChanged = true;
    std::shared_ptr<PremultipliedImage> elevationStops; // Elevation values for each stop
    std::shared_ptr<PremultipliedImage> colorStops;     // RGB colors for each stop

    // GPU textures
    std::shared_ptr<gfx::Texture2D> elevationStopsTexture;
    std::shared_ptr<gfx::Texture2D> colorStopsTexture;

    // Shader
    gfx::ShaderProgramBasePtr colorReliefShader;

    // Vertex data
    using ColorReliefVertexVector = gfx::VertexVector<HillshadeLayoutVertex>;
    std::shared_ptr<ColorReliefVertexVector> staticDataSharedVertices;
};

} // namespace mbgl
