#pragma once

#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/circle_layer_impl.hpp>
#include <mbgl/style/layers/circle_layer_properties.hpp>

namespace mbgl {

class CircleProgram;
class CircleLayerTweaker;

using CircleLayerTweakerPtr = std::shared_ptr<CircleLayerTweaker>;

class RenderCircleLayer final : public RenderLayer {
public:
    explicit RenderCircleLayer(Immutable<style::CircleLayer::Impl>);
    ~RenderCircleLayer() final = default;

#if MLN_DRAWABLE_RENDERER
    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;
#endif

private:
    void prepare(const LayerPrepareParameters&) override;
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;

    bool queryIntersectsFeature(const GeometryCoordinates&,
                                const GeometryTileFeature&,
                                float,
                                const TransformState&,
                                float,
                                const mat4&,
                                const FeatureState&) const override;


#if MLN_LEGACY_RENDERER
    void render(PaintParameters&) override;
#endif // MLN_LEGACY_RENDERER

#if MLN_DRAWABLE_RENDERER
    void updateLayerTweaker();
#endif // MLN_DRAWABLE_RENDERER

private:
    // Paint properties
    style::CirclePaintProperties::Unevaluated unevaluated;

#if MLN_LEGACY_RENDERER
    // Programs
    std::shared_ptr<CircleProgram> circleProgram;
#endif

#if MLN_DRAWABLE_RENDERER
    gfx::ShaderGroupPtr circleShaderGroup;

    CircleLayerTweakerPtr tweaker;
#if MLN_RENDER_BACKEND_METAL
    std::vector<std::string> propertiesAsUniforms;
#endif // MLN_RENDER_BACKEND_METAL

    bool overdrawInspector = false;
#endif
};

} // namespace mbgl
