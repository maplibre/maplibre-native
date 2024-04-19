#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/render_fill_layer.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/fill.hpp>
#endif

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
            /* .width = */ 1.f,
            0,
        };
        context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &propsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idFillEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

    const auto& translation = evaluated.get<FillTranslate>();
    const auto anchor = evaluated.get<FillTranslateAnchor>();

    const auto renderableSize = parameters.backend.getDefaultRenderable().getSize();
    const auto intZoom = parameters.state.getIntegerZoom();
    const auto pixelRatio = parameters.pixelRatio;

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

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
                break;
            }
            case RenderFillLayer::FillVariant::FillOutline: {
                const FillOutlineDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
                    /* pad1 */ 0,
                    /* pad2 */ 0};
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillPattern: {
                const FillPatternDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
                    /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                    /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    0,
                    0,
                };
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillOutlinePattern: {
                const FillOutlinePatternDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
                    /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
                    /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                    /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                };
                drawableUniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillOutlineTriangulated: {
                const FillOutlineTriangulatedDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.units_to_pixels=*/{1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},
                    /*.ratio=*/1.0f / tileID.pixelsToTileUnits(1.0f, parameters.state.getZoom()),
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
