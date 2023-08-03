#pragma once

#include <mbgl/programs/debug_program.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/debug_layer_impl.hpp>
#include <mbgl/style/layers/debug_layer_properties.hpp>

#include <memory>
#include <vector>

namespace mbgl {

namespace gfx {
class Drawable;
using DrawablePtr = std::shared_ptr<Drawable>;
} // namespace gfx

class ChangeRequest;

namespace style {

class RenderDebugLayer final : public RenderLayer {
public:
    explicit RenderDebugLayer(Immutable<style::DebugLayer::Impl>);
    ~RenderDebugLayer() override;

#if MLN_DRAWABLE_RENDERER
    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;
#endif

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;

    // Paint properties
    style::DebugPaintProperties::Unevaluated unevaluated;

#if MLN_DRAWABLE_RENDERER
    gfx::ShaderGroupPtr debugShaderGroup;
#endif
};

} // namespace style

} // namespace mbgl
