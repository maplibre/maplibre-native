#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/render_fill_layer.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

void FillLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    auto& context = parameters.context;
    const auto& props = static_cast<const FillLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;
    const auto& crossfade = props.crossfade;

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
        const FillEvaluatedPropsUBO propsUBO = {
            .color = evaluated.get<FillColor>().constantOr(FillColor::defaultValue()),
            .outline_color = evaluated.get<FillOutlineColor>().constantOr(FillOutlineColor::defaultValue()),
            .opacity = evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
            .fade = crossfade.t,
            .from_scale = crossfade.fromScale,
            .to_scale = crossfade.toScale,
        };
        context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &propsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idFillEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

    const auto& translation = evaluated.get<FillTranslate>();
    const auto anchor = evaluated.get<FillTranslateAnchor>();
    const auto zoom = static_cast<float>(parameters.state.getZoom());
    const auto intZoom = parameters.state.getIntegerZoom();

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<FillDrawableUnionUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<FillTilePropsUnionUBO> tilePropsUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        auto* binders = static_cast<FillBinders*>(drawable.getBinders());
        const auto* tile = drawable.getRenderTile();
        if (!binders || !tile) {
            assert(false);
            return;
        }

        const auto& fillPatternValue = evaluated.get<FillPattern>().constantOr(
            Faded<expression::Image>{.from = "", .to = ""});
        const auto patternPosA = tile->getPattern(fillPatternValue.from.id());
        const auto patternPosB = tile->getPattern(fillPatternValue.to.id());
        binders->setPatternParameters(patternPosA, patternPosB, crossfade);

        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(
            tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits, drawable);

        // from FillPatternProgram::layoutUniformValues
        const auto tileRatio = 1.0f / tileID.pixelsToTileUnits(1.0f, intZoom);
        const int32_t tileSizeAtNearestZoom = static_cast<int32_t>(
            util::tileSize_D * parameters.state.zoomScale(intZoom - tileID.canonical.z));
        const int32_t pixelX = static_cast<int32_t>(
            tileSizeAtNearestZoom *
            (tileID.canonical.x + tileID.wrap * parameters.state.zoomScale(tileID.canonical.z)));
        const int32_t pixelY = tileSizeAtNearestZoom * tileID.canonical.y;

        Size textureSize = {0, 0};
        if (const auto& tex = drawable.getTexture(idFillImageTexture)) {
            textureSize = tex->getSize();
        }

#if !MLN_UBO_CONSOLIDATION
        auto& drawableUniforms = drawable.mutableUniformBuffers();
#endif
        switch (static_cast<RenderFillLayer::FillVariant>(drawable.getType())) {
            case RenderFillLayer::FillVariant::Fill: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillDrawableUBO = {
#else
                const FillDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),

                    .color_t = std::get<0>(binders->get<FillColor>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
                    .pad1 = 0,
                    .pad2 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
#endif
                break;
            }
            case RenderFillLayer::FillVariant::FillOutline: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillOutlineDrawableUBO = {
#else
                const FillOutlineDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),

                    .outline_color_t = std::get<0>(binders->get<FillOutlineColor>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
                    .pad1 = 0,
                    .pad2 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
#endif
                break;
            }
            case RenderFillLayer::FillVariant::FillPattern: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillPatternDrawableUBO = {
#else
                const FillPatternDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),
                    .pixel_coord_upper = {static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},

                    .pixel_coord_lower = {static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    .tile_ratio = tileRatio,

                    .pattern_from_t = std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    .pattern_to_t = std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom))
                };

#if MLN_UBO_CONSOLIDATION
                tilePropsUBOVector[i].fillPatternTilePropsUBO = FillPatternTilePropsUBO {
#else
                const FillPatternTilePropsUBO tilePropsUBO = {
#endif
                    .pattern_from = patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},

                    .pattern_to = patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},

                    .texsize = {static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    .pad1 = 0, .pad2 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
                drawableUniforms.createOrUpdate(idFillTilePropsUBO, &tilePropsUBO, context);
#endif
                break;
            }
            case RenderFillLayer::FillVariant::FillOutlinePattern: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillOutlinePatternDrawableUBO = {
#else
                const FillOutlinePatternDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),
                    .pixel_coord_upper = {static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},

                    .pixel_coord_lower = {static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    .tile_ratio = tileRatio,

                    .pattern_from_t = std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    .pattern_to_t = std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    .opacity_t = std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom))
                };

#if MLN_UBO_CONSOLIDATION
                tilePropsUBOVector[i].fillOutlinePatternTilePropsUBO = FillOutlinePatternTilePropsUBO {
#else
                const FillOutlinePatternTilePropsUBO tilePropsUBO = {
#endif
                    .pattern_from = patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},

                    .pattern_to = patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},

                    .texsize = {static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    .pad1 = 0, .pad2 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
                drawableUniforms.createOrUpdate(idFillTilePropsUBO, &tilePropsUBO, context);
#endif
                break;
            }
            case RenderFillLayer::FillVariant::FillOutlineTriangulated: {
#if MLN_UBO_CONSOLIDATION
                drawableUBOVector[i].fillOutlineTriangulatedDrawableUBO = {
#else
                const FillOutlineTriangulatedDrawableUBO drawableUBO = {
#endif
                    .matrix = util::cast<float>(matrix),
                    .ratio = 1.0f / tileID.pixelsToTileUnits(1.0f, parameters.state.getZoom()),
                    .pad1 = 0,
                    .pad2 = 0,
                    .pad3 = 0
                };

#if !MLN_UBO_CONSOLIDATION
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
#endif
                break;
            }
            default: {
#ifndef NDEBUG
                mbgl::Log::Error(mbgl::Event::Render, "Invalid fill variant type supplied during drawable update!");
#endif
                break;
            }
        }
#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const size_t drawableUBOVectorSize = sizeof(FillDrawableUnionUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    const size_t tilePropsUBOVectorSize = sizeof(FillTilePropsUnionUBO) * tilePropsUBOVector.size();
    if (!tilePropsUniformBuffer || tilePropsUniformBuffer->getSize() < tilePropsUBOVectorSize) {
        tilePropsUniformBuffer = context.createUniformBuffer(
            tilePropsUBOVector.data(), tilePropsUBOVectorSize, false, true);
    } else {
        tilePropsUniformBuffer->update(tilePropsUBOVector.data(), tilePropsUBOVectorSize);
    }

    layerUniforms.set(idFillDrawableUBO, drawableUniformBuffer);
    layerUniforms.set(idFillTilePropsUBO, tilePropsUniformBuffer);
#endif
}

} // namespace mbgl
