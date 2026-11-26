#pragma once

#include <mln/shaders/segment.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/layers/background_layer_impl.hpp>
#include <mln/style/layers/background_layer_properties.hpp>

#include <optional>
#include <memory>
#include <vector>

namespace mln {

namespace gfx {
class Drawable;
using DrawablePtr = std::shared_ptr<Drawable>;
} // namespace gfx

class ChangeRequest;

class RenderBackgroundLayer final : public RenderLayer {
public:
    explicit RenderBackgroundLayer(Immutable<style::BackgroundLayer::Impl>);
    ~RenderBackgroundLayer() override;

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
    std::optional<Color> getSolidBackground() const override;

    void prepare(const LayerPrepareParameters &) override;

    // Paint properties
    style::BackgroundPaintProperties::Unevaluated unevaluated;
    SegmentVector segments;

    // Drawable shaders
    gfx::ShaderProgramBasePtr plainShader;
    gfx::ShaderProgramBasePtr patternShader;
};

} // namespace mln
