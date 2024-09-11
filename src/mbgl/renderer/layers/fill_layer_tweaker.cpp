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
    auto& context = parameters.context;
    const auto& props = static_cast<const FillLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;
    const auto& crossfade = props.crossfade;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
        const FillEvaluatedPropsUBO propsUBO = {
            /* .color = */ evaluated.get<FillColor>().constantOr(FillColor::defaultValue()),
            /* .outline_color = */ evaluated.get<FillOutlineColor>().constantOr(FillOutlineColor::defaultValue()),
            /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
            /* .fade = */ crossfade.t,
            /* .from_scale = */ crossfade.fromScale,
            /* .to_scale = */ crossfade.toScale,
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

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        auto* binders = static_cast<FillProgram::Binders*>(drawable.getBinders());
        const auto* tile = drawable.getRenderTile();
        if (!binders || !tile) {
            assert(false);
            return;
        }

        const auto& fillPatternValue = evaluated.get<FillPattern>().constantOr(Faded<expression::Image>{"", ""});
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

        auto& drawableUniforms = drawable.mutableUniformBuffers();

        switch (static_cast<RenderFillLayer::FillVariant>(drawable.getType())) {
            case RenderFillLayer::FillVariant::Fill: {
                const FillDrawableUBO drawableUBO = {/*.matrix=*/util::cast<float>(matrix)};
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);

                const auto fillInterpolateUBO = FillInterpolateUBO{
                    /* .color_t = */ std::get<0>(binders->get<FillColor>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                    0,
                };
                drawableUniforms.createOrUpdate(idFillInterpolateUBO, &fillInterpolateUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillOutline: {
                const FillOutlineDrawableUBO drawableUBO = {/*.matrix=*/util::cast<float>(matrix)};
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);

                const auto fillOutlineInterpolateUBO = FillOutlineInterpolateUBO{
                    /* .color_t = */ std::get<0>(binders->get<FillOutlineColor>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                    0,
                };
                drawableUniforms.createOrUpdate(idFillInterpolateUBO, &fillOutlineInterpolateUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillPattern: {
                const FillPatternDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                    /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    /*.tile_ratio = */ tileRatio,
                    0,
                };
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);

                const auto fillPatternInterpolateUBO = FillPatternInterpolateUBO{
                    /* .pattern_from_t = */ std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    /* .pattern_to_t = */ std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                };
                drawableUniforms.createOrUpdate(idFillInterpolateUBO, &fillPatternInterpolateUBO, context);

                const auto fillPatternTilePropsUBO = FillPatternTilePropsUBO{
                    /* pattern_from = */ patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
                    /* pattern_to = */ patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},
                };
                drawableUniforms.createOrUpdate(idFillTilePropsUBO, &fillPatternTilePropsUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillOutlinePattern: {
                const FillOutlinePatternDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                    /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    /*.tile_ratio = */ tileRatio,
                    0};
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);

                const auto fillOutlinePatternInterpolateUBO = FillPatternInterpolateUBO{
                    /* .pattern_from_t = */ std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    /* .pattern_to_t = */ std::get<0>(binders->get<FillPattern>()->interpolationFactor(zoom)),
                    /* .opacity_t = */ std::get<0>(binders->get<FillOpacity>()->interpolationFactor(zoom)),
                    0,
                };
                drawableUniforms.createOrUpdate(idFillInterpolateUBO, &fillOutlinePatternInterpolateUBO, context);

                const auto fillOutlinePatternTilePropsUBO = FillOutlinePatternTilePropsUBO{
                    /* pattern_from = */ patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
                    /* pattern_to = */ patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},
                };
                drawableUniforms.createOrUpdate(idFillTilePropsUBO, &fillOutlinePatternTilePropsUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillOutlineTriangulated: {
                const FillOutlineTriangulatedDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.ratio=*/1.0f / tileID.pixelsToTileUnits(1.0f, parameters.state.getZoom()),
                    0,
                    0,
                    0};
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
                break;
            }
            default: {
#ifndef NDEBUG
                mbgl::Log::Error(mbgl::Event::Render, "Invalid fill variant type supplied during drawable update!");
#endif
                break;
            }
        }
    });
}

} // namespace mbgl
