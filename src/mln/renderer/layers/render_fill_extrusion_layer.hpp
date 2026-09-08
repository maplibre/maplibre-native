#pragma once

#include <mln/renderer/render_layer.hpp>
#include <mln/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mln/style/layers/fill_extrusion_layer_impl.hpp>
#include <mln/style/layers/fill_extrusion_layer_properties.hpp>

namespace mln {

class RenderFillExtrusionLayer final : public RenderLayer {
public:
    explicit RenderFillExtrusionLayer(Immutable<style::FillExtrusionLayer::Impl>);
    ~RenderFillExtrusionLayer() override;

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;
    bool is3D() const override;

    void markLayerRenderable(bool willRender, UniqueChangeRequestVec&) override;
    void layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec&) override;
    std::size_t removeAllDrawables() override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const PaintParameters&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;

    // Paint properties
    style::FillExtrusionPaintProperties::Unevaluated unevaluated;

    gfx::ShaderGroupPtr fillExtrusionGroup;
    gfx::ShaderGroupPtr fillExtrusionPatternGroup;

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    gfx::ShaderGroupPtr fillExtrusionInstancedGroup;
    gfx::ShaderGroupPtr fillExtrusionPatternInstancedGroup;

    using FillExtrusionVertexVector = gfx::VertexVector<FillExtrusionStaticVertex>;
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;

    std::shared_ptr<FillExtrusionVertexVector> staticDataVertices;
    std::shared_ptr<TriangleIndexVector> staticDataIndices;
    std::shared_ptr<SegmentVector> staticDataSegments;

    // Ground shadows. Each building's roof/wall geometry is sheared along the light direction and
    // redrawn as flat, alpha-blended ground-plane geometry, positioned with the exact same per-tile
    // camera matrix as the building itself (see getTileMatrix() in the tweaker). Registered ahead
    // of the building layer group at the same layer index (insertion-order trick, see
    // markLayerRenderable()) so shadows draw underneath the buildings. Only the instanced
    // (Metal/Vulkan) geometry layout is supported, and the shaders are currently only specialized
    // for Metal/Vulkan -- on other backends the shader lookup simply fails and the whole feature
    // stays switched off.

    /// Whether the evaluated properties ask for a shadow at all.
    bool shadowEnabled() const;

    /// Create the shadow's tile layer group and shader groups. Returns false if any resource could
    /// not be obtained, in which case the caller should skip the shadow entirely.
    bool prepareShadow(gfx::ShaderRegistry&, gfx::Context&, UniqueChangeRequestVec&);

    /// Release every shadow resource and deregister it from the orchestrator.
    void teardownShadow(UniqueChangeRequestVec&);

    /// Registered at the same layerIndex as `layerGroup` but *before* it, so it draws underneath
    /// the buildings.
    LayerGroupBasePtr shadowGroup;

    gfx::ShaderGroupPtr shadowMaskShaderGroup;
    gfx::ShaderGroupPtr shadowMaskInstancedShaderGroup;

    LayerTweakerPtr shadowTweaker;

    /// Tracks whether the shadow was active last update, so that toggling it forces a full drawable
    /// rebuild. Without this, tiles that already have building drawables get skipped by updateTile
    /// and would never gain their shadow drawables.
    bool shadowWasEnabled = false;

    // Throttled cost reporting, enabled with the MLN_SHADOW_STATS environment variable.
    void reportShadowStats(double setupMs);
    std::uint64_t shadowFrameCount = 0;
    double shadowSetupMsAccum = 0.0;
#endif
};

} // namespace mln
