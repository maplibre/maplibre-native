#pragma once

#include <mbgl/text/glyph.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/style/image_impl.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>

#include <unordered_map>

namespace mbgl {

namespace style {

// Repackaging evaluated values from SymbolLayoutProperties +
// SymbolPaintProperties for genericity over icons vs. text.
class SymbolPropertyValues {
public:
    // Layout
    AlignmentType pitchAlignment;
    AlignmentType rotationAlignment;
    bool keepUpright;

    // Paint
    std::array<float, 2> translate;
    TranslateAnchorType translateAnchor;

    bool hasHalo;
    bool hasFill;
};

enum class SymbolType : uint8_t {
    Text,
    IconRGBA,
    IconSDF
};

} // namespace style

class SymbolLayerTweaker;
using SymbolLayerTweakerPtr = std::shared_ptr<SymbolLayerTweaker>;

class RenderSymbolLayer final : public RenderLayer {
public:
    explicit RenderSymbolLayer(Immutable<style::SymbolLayer::Impl>);
    ~RenderSymbolLayer() override;

    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const std::shared_ptr<UpdateParameters>&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;

    /// Remove all the drawables for tiles
    std::size_t removeAllDrawables() override;

protected:
    /// @brief Called by the RenderOrchestrator during RenderTree construction.
    /// This event is run to indicate if the layer should render or not for the current frame.
    /// @param willRender Indicates if this layer should render or not
    /// @param changes The collection of current pending change requests
    void markLayerRenderable(bool willRender, UniqueChangeRequestVec&) override;

    /// @brief Called when the layer index changes
    /// This event is run when a layer is added or removed from the style.
    /// @param newLayerIndex The new layer index for this layer
    /// @param changes The collection of current pending change requests
    void layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec&) override;

    /// Called when the style layer is removed
    void layerRemoved(UniqueChangeRequestVec&) override;

    /// Remove all drawables for the tile from the layer group
    std::size_t removeTile(RenderPass, const OverscaledTileID&) override;

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;

    void prepare(const LayerPrepareParameters&) override;

private:
    // Paint properties
    style::SymbolPaintProperties::Unevaluated unevaluated;

    float iconSize = 1.0f;
    float textSize = 16.0f;

    bool hasFormatSectionOverrides = false;

    gfx::ShaderGroupPtr symbolIconGroup;
    gfx::ShaderGroupPtr symbolSDFGroup;
    gfx::ShaderGroupPtr symbolTextAndIconGroup;

    gfx::ShaderGroupPtr collisionBoxGroup;
    gfx::ShaderGroupPtr collisionCircleGroup;
    std::shared_ptr<TileLayerGroup> collisionTileLayerGroup;

    LayerTweakerPtr collisionLayerTweaker;

    using SymbolVertexVector = gfx::VertexVector<SymbolStaticVertex>;
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;

    std::shared_ptr<SymbolVertexVector> staticDataVertices;
    std::shared_ptr<TriangleIndexVector> staticDataIndices;
    // std::shared_ptr<SegmentVector> staticDataSegments;
};

} // namespace mbgl
