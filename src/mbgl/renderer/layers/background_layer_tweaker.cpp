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

#ifndef NDEBUG
constexpr auto BackgroundPatternShaderName = "BackgroundPatternShader";
#endif

void BackgroundLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    const auto& state = parameters.state;
    auto& context = parameters.context;

#ifdef DEBUG
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
        const BackgroundPatternPropsUBO propsUBO = {.pattern_tl_a = util::cast<float>(imagePosA->tl()),
                                                    .pattern_br_a = util::cast<float>(imagePosA->br()),
                                                    .pattern_tl_b = util::cast<float>(imagePosB->tl()),
                                                    .pattern_br_b = util::cast<float>(imagePosB->br()),
                                                    .pattern_size_a = imagePosA->displaySize(),
                                                    .pattern_size_b = imagePosB->displaySize(),
                                                    .scale_a = crossfade.fromScale,
                                                    .scale_b = crossfade.toScale,
                                                    .mix = crossfade.t,
                                                    .opacity = evaluated.get<BackgroundOpacity>()};
        layerUniforms.createOrUpdate(idBackgroundPropsUBO, &propsUBO, context);
    } else {
        const BackgroundPropsUBO propsUBO = {.color = evaluated.get<BackgroundColor>(),
                                             .opacity = evaluated.get<BackgroundOpacity>(),
                                             .pad1 = 0,
                                             .pad2 = 0,
                                             .pad3 = 0};
        layerUniforms.createOrUpdate(idBackgroundPropsUBO, &propsUBO, context);
    }

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<BackgroundDrawableUnionUBO> drawableUBOVector(layerGroup.getDrawableCount());
#endif

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

#if !MLN_UBO_CONSOLIDATION
        auto& drawableUniforms = drawable.mutableUniformBuffers();
#endif

        if (hasPattern) {
            if (const auto& tex = parameters.patternAtlas.texture()) {
                tex->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                              .wrapU = gfx::TextureWrapType::Clamp,
                                              .wrapV = gfx::TextureWrapType::Clamp});
                drawable.setTexture(tex, idBackgroundImageTexture);
            }

            const int32_t tileSizeAtNearestZoom = static_cast<int32_t>(
                util::tileSize_D * state.zoomScale(state.getIntegerZoom() - tileID.canonical.z));
            const int32_t pixelX = static_cast<int32_t>(
                tileSizeAtNearestZoom * (tileID.canonical.x + tileID.wrap * state.zoomScale(tileID.canonical.z)));
            const int32_t pixelY = tileSizeAtNearestZoom * tileID.canonical.y;
            const auto pixToTile = tileID.pixelsToTileUnits(1.0f, state.getIntegerZoom());

#if MLN_UBO_CONSOLIDATION
            drawableUBOVector[i].backgroundPatternDrawableUBO = {
#else
            const BackgroundPatternDrawableUBO drawableUBO = {
#endif
                .matrix = util::cast<float>(matrix),
                .pixel_coord_upper = {static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                .pixel_coord_lower = {static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                .tile_units_to_pixels = (pixToTile != 0) ? 1.0f / pixToTile : 0.0f,
                .pad1 = 0,
                .pad2 = 0,
                .pad3 = 0
            };
#if !MLN_UBO_CONSOLIDATION
            drawableUniforms.createOrUpdate(idBackgroundDrawableUBO, &drawableUBO, context);
#endif
        } else {

#if MLN_UBO_CONSOLIDATION
            drawableUBOVector[i].backgroundDrawableUBO = {
#else
            const BackgroundDrawableUBO drawableUBO = {
#endif
                util::cast<float>(matrix)
            };
#if !MLN_UBO_CONSOLIDATION
            drawableUniforms.createOrUpdate(idBackgroundDrawableUBO, &drawableUBO, context);
#endif
        }

#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const size_t drawableUBOVectorSize = sizeof(BackgroundDrawableUnionUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    layerUniforms.set(idBackgroundDrawableUBO, drawableUniformBuffer);
#endif
}

} // namespace mbgl
