#pragma once

#include <mbgl/programs/background_program.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/layers/background_layer_impl.hpp>
#include <mbgl/style/layers/background_layer_properties.hpp>

#include <optional>
#include <memory>
#include <vector>

namespace mbgl {

namespace gfx {
class Drawable;
using DrawablePtr = std::shared_ptr<Drawable>;
} // namespace gfx

class ChangeRequest;

class RenderBackgroundLayer final : public RenderLayer {
public:
    explicit RenderBackgroundLayer(Immutable<style::BackgroundLayer::Impl>);
    ~RenderBackgroundLayer() override;

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
    std::optional<Color> getSolidBackground() const override;
    void render(PaintParameters&) override;
    void prepare(const LayerPrepareParameters&) override;

    // Paint properties
    style::BackgroundPaintProperties::Unevaluated unevaluated;
    SegmentVector<BackgroundAttributes> segments;

    // Programs
    std::shared_ptr<BackgroundProgram> backgroundProgram;
    std::shared_ptr<BackgroundPatternProgram> backgroundPatternProgram;
};

struct alignas(16) BackgroundLayerUBO {
    Color color;
    float opacity;
    float padding[3];
};
static_assert(sizeof(BackgroundLayerUBO) % 16 == 0);

struct alignas(16) BackgroundPatternLayerUBO {
    std::array<float, 2> pattern_tl_a;
    std::array<float, 2> pattern_br_a;
    std::array<float, 2> pattern_tl_b;
    std::array<float, 2> pattern_br_b;
    std::array<float, 2> texsize;
    std::array<float, 2> pattern_size_a;
    std::array<float, 2> pattern_size_b;
    std::array<float, 2> pixel_coord_upper;
    std::array<float, 2> pixel_coord_lower;
    float tile_units_to_pixels;
    float scale_a;
    float scale_b;
    float mix;
    float opacity;
};
static_assert(sizeof(BackgroundPatternLayerUBO) % 16 == 0);

} // namespace mbgl
