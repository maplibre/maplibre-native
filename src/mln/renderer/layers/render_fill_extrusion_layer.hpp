#pragma once

#include <mln/renderer/render_layer.hpp>
#include <mln/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mln/style/layers/fill_extrusion_layer_impl.hpp>
#include <mln/style/layers/fill_extrusion_layer_properties.hpp>
#include <mln/shaders/layer_ubo.hpp>

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

    void markLayerRenderable(bool willRender, UniqueChangeRequestVec &) override;
    void layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec &) override;
    void layerRemoved(UniqueChangeRequestVec &) override;
    std::size_t removeAllDrawables() override;

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

    // Paint properties
    style::FillExtrusionPaintProperties::Unevaluated unevaluated;

    gfx::ShaderGroupPtr fillExtrusionGroup;
    gfx::ShaderGroupPtr fillExtrusionPatternGroup;

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    gfx::ShaderGroupPtr fillExtrusionInstancedGroup;
    gfx::ShaderGroupPtr fillExtrusionPatternInstancedGroup;
#endif

#if MLN_USE_FILL_EXTRUSION_GROUND_OCCLUSION
    using FillExtrusionVertexVector = gfx::VertexVector<FillExtrusionStaticVertex>;
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;

    std::shared_ptr<FillExtrusionVertexVector> staticDataVertices;
    std::shared_ptr<TriangleIndexVector> staticDataIndices;
#endif

    // Ground ambient occlusion. Flat quads around each footprint, drawn from the bucket's outline
    // with the same per-tile camera matrix as the buildings (see the ground occlusion tweaker).
    // Registered after the building layer group at the same layer index, so it is depth tested
    // against the buildings (see markLayerRenderable()).

    /// Whether the style can ask for ambient occlusion on the ground at some zoom or point of a
    /// transition.
    bool groundOcclusionWanted() const;

    /// Whether the framebuffer is opaque under the layer, which the ground ambient occlusion needs.
    /// Warns once when it is not.
    bool hasOpaqueBackdrop(const PaintParameters &, const RenderTree &);

    /// Create the ground ambient occlusion layer group and shader groups. Returns false on backends
    /// without the shaders, which then draw no ground ambient occlusion.
    bool prepareGroundOcclusion(gfx::ShaderRegistry &, gfx::Context &, UniqueChangeRequestVec &);

    /// Release the ground ambient occlusion layer group and deregister it from the orchestrator.
    void teardownGroundOcclusion(UniqueChangeRequestVec &);

    /// Re-registers the ground ambient occlusion group after the buildings
    void keepGroundOcclusionLast(UniqueChangeRequestVec &);

    gfx::ShaderGroupPtr groundOcclusionShaderGroup;
    gfx::ShaderGroupPtr groundOcclusionResolveShaderGroup;

    /// Its own group because its drawables are flat and clipped to their tiles by the stencil,
    /// while the buildings are 3D.
    LayerGroupBasePtr groundOcclusionGroup;
    LayerTweakerPtr groundOcclusionTweaker;

    /// Tracks whether the ground ambient occlusion was active last update, so that toggling it
    /// forces a full drawable rebuild. Without this, tiles that already have building drawables get
    /// skipped by updateTile() and would never gain their occlusion drawables. This follows the
    /// style, not the zoom: a value that is zero only at the current zoom disables the group instead.
    bool groundOcclusionWasEnabled = false;
    /// Whether hasOpaqueBackdrop() has warned already
    bool groundOcclusionBackdropWarned = false;
};

} // namespace mln
