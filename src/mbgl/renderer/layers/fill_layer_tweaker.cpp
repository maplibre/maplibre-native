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
#include <mbgl/util/string_indexer.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/fill.hpp>
#endif

namespace mbgl {

using namespace style;

static const StringIdentity idTexImageName = stringIndexer().get("u_image");
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

    if (propertiesUpdated) {
        fillUniformBufferUpdated = true;
        fillOutlineUniformBufferUpdated = true;
        fillPatternUniformBufferUpdated = true;
        fillOutlinePatternUniformBufferUpdated = true;
        propertiesUpdated = false;
    }

    const auto UpdateFillUniformBuffers = [&]() {
        if (!fillPropsUniformBuffer || fillUniformBufferUpdated) {
            const FillEvaluatedPropsUBO paramsUBO = {
                /* .color = */ evaluated.get<FillColor>().constantOr(FillColor::defaultValue()),
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                0,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillPropsUniformBuffer, &paramsUBO);
            fillUniformBufferUpdated = false;
        }
    };

    const auto UpdateFillOutlineUniformBuffers = [&]() {
        if (!fillOutlinePropsUniformBuffer || fillOutlineUniformBufferUpdated) {
            const FillOutlineEvaluatedPropsUBO paramsUBO = {
                /* .outline_color = */ evaluated.get<FillOutlineColor>().constantOr(FillOutlineColor::defaultValue()),
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                0,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillOutlinePropsUniformBuffer, &paramsUBO);
            fillOutlineUniformBufferUpdated = false;
        }
    };

    const auto UpdateFillPatternUniformBuffers = [&]() {
        if (!fillPatternPropsUniformBuffer || fillPatternUniformBufferUpdated) {
            const FillPatternEvaluatedPropsUBO paramsUBO = {
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                /* .fade = */ crossfade.t,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillPatternPropsUniformBuffer, &paramsUBO);
            fillPatternUniformBufferUpdated = false;
        }
    };

    const auto UpdateFillOutlinePatternUniformBuffers = [&]() {
        if (!fillOutlinePatternPropsUniformBuffer || fillOutlinePatternUniformBufferUpdated) {
            const FillOutlinePatternEvaluatedPropsUBO paramsUBO = {
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                /* .fade = */ crossfade.t,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillOutlinePatternPropsUniformBuffer, &paramsUBO);
            fillOutlinePatternUniformBufferUpdated = false;
        }
    };

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
        if (const auto shader = drawable.getShader()) {
            if (const auto index = shader->getSamplerLocation(idTexImageName)) {
                if (const auto& tex = drawable.getTexture(*index)) {
                    textureSize = tex->getSize();
                }
            }
        }

        auto& uniforms = drawable.mutableUniformBuffers();
        switch (static_cast<RenderFillLayer::FillVariant>(drawable.getType())) {
            case RenderFillLayer::FillVariant::Fill: {
                UpdateFillUniformBuffers();

                uniforms.set(idFillEvaluatedPropsUBO, fillPropsUniformBuffer);

                const FillDrawableUBO drawableUBO = {/*.matrix=*/util::cast<float>(matrix)};
                uniforms.createOrUpdate(idFillDrawableUBO, &drawableUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillOutline: {
                UpdateFillOutlineUniformBuffers();

                uniforms.set(idFillOutlineEvaluatedPropsUBO, fillOutlinePropsUniformBuffer);

                const FillOutlineDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
                    /* pad1 */ 0,
                    /* pad2 */ 0};
                uniforms.createOrUpdate(idFillOutlineDrawableUBO, &drawableUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillPattern: {
                UpdateFillPatternUniformBuffers();

                uniforms.set(idFillPatternEvaluatedPropsUBO, fillPatternPropsUniformBuffer);

                const FillPatternDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
                    /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                    /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                    0,
                    0,
                };
                uniforms.createOrUpdate(idFillPatternDrawableUBO, &drawableUBO, context);
                break;
            }
            case RenderFillLayer::FillVariant::FillOutlinePattern: {
                UpdateFillOutlinePatternUniformBuffers();

                uniforms.set(idFillOutlinePatternEvaluatedPropsUBO, fillOutlinePatternPropsUniformBuffer);

                const FillOutlinePatternDrawableUBO drawableUBO = {
                    /*.matrix=*/util::cast<float>(matrix),
                    /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
                    /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
                    /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                    /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                    /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                };
                uniforms.createOrUpdate(idFillOutlinePatternDrawableUBO, &drawableUBO, context);
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
