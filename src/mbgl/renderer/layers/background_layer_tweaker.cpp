#include <mbgl/renderer/layers/background_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/pattern_atlas.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/background_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

#if !defined(NDEBUG)
constexpr auto BackgroundPatternShaderName = "BackgroundPatternShader";
#endif

void BackgroundLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    const auto& state = parameters.state;
    auto& context = parameters.context;

    if (layerGroup.empty()) {
        return;
    }

#if defined(DEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    const auto& evaluated = static_cast<const BackgroundLayerProperties&>(*evaluatedProperties).evaluated;
    const auto& crossfade = static_cast<const BackgroundLayerProperties&>(*evaluatedProperties).crossfade;
    const bool hasPattern = !evaluated.get<BackgroundPattern>().to.empty();
    const auto imagePosA = hasPattern ? parameters.patternAtlas.getPattern(evaluated.get<BackgroundPattern>().from.id())
                                      : std::nullopt;
    const auto imagePosB = hasPattern ? parameters.patternAtlas.getPattern(evaluated.get<BackgroundPattern>().to.id())
                                      : std::nullopt;

    if (hasPattern && (!imagePosA || !imagePosB)) {
        // The pattern isn't valid, disable the whole thing.
        layerGroup.setEnabled(false);
        return;
    }
    layerGroup.setEnabled(true);

    // properties are re-evaluated every time
    propertiesUpdated = false;

    auto& layerUniforms = layerGroup.mutableUniformBuffers();

    if (hasPattern) {
        const Size atlasSize = parameters.patternAtlas.getPixelSize();

        const BackgroundPatternLayerUBO layerUBO = {
            /* .pattern_tl_a = */ util::cast<float>(imagePosA->tl()),
            /* .pattern_br_a = */ util::cast<float>(imagePosA->br()),
            /* .pattern_tl_b = */ util::cast<float>(imagePosB->tl()),
            /* .pattern_br_b = */ util::cast<float>(imagePosB->br()),
            /* .texsize = */ {static_cast<float>(atlasSize.width), static_cast<float>(atlasSize.height)},
            /* .pattern_size_a = */ imagePosA->displaySize(),
            /* .pattern_size_b = */ imagePosB->displaySize(),
            /* .scale_a = */ crossfade.fromScale,
            /* .scale_b = */ crossfade.toScale,
            /* .mix = */ crossfade.t,
            /* .opacity = */ evaluated.get<BackgroundOpacity>(),
            /* .pad1/2 = */ 0,
            0};
        layerUniforms.createOrUpdate(idBackgroundLayerUBO, &layerUBO, context);
    } else {
        const BackgroundLayerUBO layerUBO = {/* .color = */ evaluated.get<BackgroundColor>(),
                                             /* .opacity = */ evaluated.get<BackgroundOpacity>(),
                                             /* .pad1/2/3 = */ 0,
                                             0,
                                             0};
        layerUniforms.createOrUpdate(idBackgroundLayerUBO, &layerUBO, context);
    }

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        assert(drawable.getTileID());
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        // We assume that drawables don't change between pattern and non-pattern.
        assert(hasPattern == (drawable.getShader() ==
                              context.getGenericShader(parameters.shaders, std::string(BackgroundPatternShaderName))));

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto matrix = getTileMatrix(
            tileID, parameters, {0.f, 0.f}, TranslateAnchorType::Viewport, false, false, drawable);

        auto& drawableUniforms = drawable.mutableUniformBuffers();

        if (hasPattern) {
            if (const auto& tex = parameters.patternAtlas.texture()) {
                tex->setSamplerConfiguration(
                    {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
                drawable.setTexture(tex, idBackgroundImageTexture);
            }

            const int32_t tileSizeAtNearestZoom = static_cast<int32_t>(
                util::tileSize_D * state.zoomScale(state.getIntegerZoom() - tileID.canonical.z));
            const int32_t pixelX = static_cast<int32_t>(
                tileSizeAtNearestZoom * (tileID.canonical.x + tileID.wrap * state.zoomScale(tileID.canonical.z)));
            const int32_t pixelY = tileSizeAtNearestZoom * tileID.canonical.y;
            const auto pixToTile = tileID.pixelsToTileUnits(1.0f, state.getIntegerZoom());

            const BackgroundPatternDrawableUBO drawableUBO = {
                /* .matrix = */ util::cast<float>(matrix),
                /* .pixel_coord_upper = */ {static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                /* .pixel_coord_lower = */ {static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                /* .tile_units_to_pixels = */ (pixToTile != 0) ? 1.0f / pixToTile : 0.0f,
                /* .pad1/2/3 = */ 0,
                0,
                0};
            drawableUniforms.createOrUpdate(idBackgroundDrawableUBO, &drawableUBO, context);
        } else {
            const BackgroundDrawableUBO drawableUBO = {/* .matrix = */ util::cast<float>(matrix)};
            drawableUniforms.createOrUpdate(idBackgroundDrawableUBO, &drawableUBO, context);
        }
    });
}

} // namespace mbgl
