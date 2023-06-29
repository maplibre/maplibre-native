#pragma once

#include <mbgl/text/glyph.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/style/image_impl.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>

#if MLN_DRAWABLE_RENDERER
#include <unordered_map>
#endif // MLN_DRAWABLE_RENDERER

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

class SymbolIconProgram;
class SymbolSDFIconProgram;
class SymbolSDFTextProgram;
class SymbolTextAndIconProgram;
class CollisionBoxProgram;
class CollisionCircleProgram;

class RenderSymbolLayer final : public RenderLayer {
public:
    struct Programs {
        std::shared_ptr<SymbolIconProgram> symbolIconProgram;
        std::shared_ptr<SymbolSDFIconProgram> symbolSDFIconProgram;
        std::shared_ptr<SymbolSDFTextProgram> symbolSDFTextProgram;
        std::shared_ptr<SymbolTextAndIconProgram> symbolTextAndIconProgram;
        std::shared_ptr<CollisionBoxProgram> collisionBoxProgram;
        std::shared_ptr<CollisionCircleProgram> collisionCircleProgram;
    };

public:
    explicit RenderSymbolLayer(Immutable<style::SymbolLayer::Impl>);
    ~RenderSymbolLayer() override;

    static style::IconPaintProperties::PossiblyEvaluated iconPaintProperties(
        const style::SymbolPaintProperties::PossiblyEvaluated&);
    static style::TextPaintProperties::PossiblyEvaluated textPaintProperties(
        const style::SymbolPaintProperties::PossiblyEvaluated&);

#if MLN_DRAWABLE_RENDERER
    /// Generate any changes needed by the layer
    void update(gfx::ShaderRegistry&,
                gfx::Context&,
                const TransformState&,
                const RenderTree&,
                UniqueChangeRequestVec&) override;
#endif // MLN_DRAWABLE_RENDERER

private:
    void transition(const TransitionParameters&) override;
    void evaluate(const PropertyEvaluationParameters&) override;
    bool hasTransition() const override;
    bool hasCrossfade() const override;

#if MLN_LEGACY_RENDERER
    void render(PaintParameters&) override;
#endif // MLN_LEGACY_RENDERER

    void prepare(const LayerPrepareParameters&) override;

private:
    // Paint properties
    style::SymbolPaintProperties::Unevaluated unevaluated;

    float iconSize = 1.0f;
    float textSize = 16.0f;

    bool hasFormatSectionOverrides = false;

#if MLN_LEGACY_RENDERER
    // Programs
    Programs programs;
#endif // MLN_LEGACY_RENDERER

#if MLN_DRAWABLE_RENDERER
    gfx::ShaderGroupPtr symbolIconGroup;
    gfx::ShaderGroupPtr symbolSDFIconGroup;
    gfx::ShaderGroupPtr symbolSDFTextGroup;
    gfx::ShaderGroupPtr symbolTextAndIconGroup;

    std::unordered_map<OverscaledTileID, uint32_t> tileBucketInstances;
#endif // MLN_DRAWABLE_RENDERER
};

} // namespace mbgl
