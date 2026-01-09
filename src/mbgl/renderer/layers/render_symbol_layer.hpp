#pragma once

#include <mbgl/text/glyph.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/image_impl.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>

#include <unordered_map>

namespace mbgl {

namespace style {

// {icon,text}-specific paint-property packs for use in the symbol Programs.
// Since each program deals either with icons or text, using a smaller property set
// lets us avoid unnecessarily binding attributes for properties the program wouldn't use.
class IconPaintProperties : public Properties<IconOpacity,
                                              IconColor,
                                              IconHaloColor,
                                              IconHaloWidth,
                                              IconHaloBlur,
                                              IconTranslate,
                                              IconTranslateAnchor> {};

class TextPaintProperties : public Properties<TextOpacity,
                                              TextColor,
                                              TextHaloColor,
                                              TextHaloWidth,
                                              TextHaloBlur,
                                              TextTranslate,
                                              TextTranslateAnchor> {};

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

    static style::IconPaintProperties::PossiblyEvaluated iconPaintProperties(
        const style::SymbolPaintProperties::PossiblyEvaluated&);
    static style::TextPaintProperties::PossiblyEvaluated textPaintProperties(
        const style::SymbolPaintProperties::PossiblyEvaluated&);

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
};

} // namespace mbgl
