#pragma once

#include <mln/renderer/render_layer.hpp>
#include <mln/renderer/layers/fill_extrusion_layer_tweaker.hpp>
#include <mln/gfx/texture2d.hpp>
#include <mln/util/image.hpp>
#include <mln/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mln/style/layers/fill_extrusion_layer_impl.hpp>
#include <mln/style/layers/fill_extrusion_layer_properties.hpp>

namespace mln {

class RenderFillExtrusionLayer final : public RenderLayer {
public:
    explicit RenderFillExtrusionLayer(Immutable<style::FillExtrusionLayer::Impl>);
    ~RenderFillExtrusionLayer() override;

private:
    void transition(const TransitionParameters &) override;
    void evaluate(const PropertyEvaluationParameters &) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    bool is3D() const override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry &,
                gfx::Context &,
                const TransformState &,
                const std::shared_ptr<UpdateParameters> &,
                const PaintParameters &,
                const RenderTree &,
                UniqueChangeRequestVec &) override;

    bool queryIntersectsFeature(const GeometryCoordinates &,
                                const GeometryTileFeature &,
                                float,
                                const TransformState &,
                                float,
                                const mat4 &,
                                const FeatureState &) const override;

    void captureRenderedFeatures(const FillExtrusionBucket &,
                                 const RenderTile &,
                                 const FillExtrusionBinders &,
                                 const style::FillExtrusionPaintProperties::PossiblyEvaluated &,
                                 const TransformState &,
                                 const TransformParameters &);

    /// Re-sample `fill-extrusion-color` into `colorRamp` if it changed and uses `["elevation"]`, or if it
    /// also depends on zoom and the zoom moved to other stops.
    void updateColorRamp(float zoom);

    // Paint properties
    style::FillExtrusionPaintProperties::Unevaluated unevaluated;

    /// `fill-extrusion-color` sampled over height, when it is an expression over `["elevation"]`.
    std::shared_ptr<PremultipliedImage> colorRamp;
    std::optional<style::PropertyValue<Color>> colorRampValue;
    ElevationColorRampZoom colorRampZoom;
    gfx::Texture2DPtr colorRampTexture;
    bool colorRampChanged = true;

    gfx::ShaderGroupPtr fillExtrusionGroup;
    gfx::ShaderGroupPtr fillExtrusionPatternGroup;

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    gfx::ShaderGroupPtr fillExtrusionInstancedGroup;
    gfx::ShaderGroupPtr fillExtrusionPatternInstancedGroup;

    using FillExtrusionVertexVector = gfx::VertexVector<FillExtrusionStaticVertex>;
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;

    std::shared_ptr<FillExtrusionVertexVector> staticDataVertices;
    std::shared_ptr<TriangleIndexVector> staticDataIndices;
#endif
};

} // namespace mln
