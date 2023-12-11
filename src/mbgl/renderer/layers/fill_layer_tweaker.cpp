#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/renderer/layer_group.hpp>
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

static const StringIdentity idFillDrawableUBOName = stringIndexer().get("FillDrawableUBO");
static const StringIdentity idFillDrawablePropsUBOName = stringIndexer().get("FillDrawablePropsUBO");
static const StringIdentity idFillEvaluatedPropsUBOName = stringIndexer().get("FillEvaluatedPropsUBO");

const StringIdentity FillLayerTweaker::idFillTilePropsUBOName = stringIndexer().get("FillDrawableTilePropsUBO");
const StringIdentity FillLayerTweaker::idFillInterpolateUBOName = stringIndexer().get("FillInterpolateUBO");
const StringIdentity FillLayerTweaker::idFillOutlineInterpolateUBOName = stringIndexer().get(
    "FillOutlineInterpolateUBO");

static const StringIdentity idFillOutlineDrawableUBOName = stringIndexer().get("FillOutlineDrawableUBO");
static const StringIdentity idFillOutlineEvaluatedPropsUBOName = stringIndexer().get("FillOutlineEvaluatedPropsUBO");

static const StringIdentity idFillOutlineInterpolateUBOName = stringIndexer().get("FillOutlineInterpolateUBO");

static const StringIdentity idFillPatternDrawableUBOName = stringIndexer().get("FillPatternDrawableUBO");
static const StringIdentity idFillPatternInterpolateUBOName = stringIndexer().get("FillPatternInterpolateUBO");
static const StringIdentity idFillPatternEvaluatedPropsUBOName = stringIndexer().get("FillPatternEvaluatedPropsUBO");
static const StringIdentity idFillPatternTilePropsUBOName = stringIndexer().get("FillPatternTilePropsUBO");

static const StringIdentity idFillOutlinePatternDrawableUBOName = stringIndexer().get("FillOutlinePatternDrawableUBO");
static const StringIdentity idFillOutlinePatternInterpolateUBOName = stringIndexer().get(
    "FillOutlinePatternInterpolateUBO");
static const StringIdentity idFillOutlinePatternEvaluatedPropsUBOName = stringIndexer().get(
    "FillOutlinePatternEvaluatedPropsUBO");
static const StringIdentity idFillOutlinePatternTilePropsUBOName = stringIndexer().get(
    "FillOutlinePatternTilePropsUBO");

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

    // Only run each update function once
    bool fillUniformBufferUpdated = false;
    bool fillOutlineUniformBufferUpdated = false;
    bool fillPatternUniformBufferUpdated = false;
    bool fillOutlinePatternUniformBufferUpdated = false;

    const auto UpdateFillUniformBuffers = [&]() {
        if (fillUniformBufferUpdated) return;
        fillUniformBufferUpdated = true;

        if (!fillPropsUniformBuffer || propertiesUpdated) {
            const FillEvaluatedPropsUBO paramsUBO = {
                /* .color = */ evaluated.get<FillColor>().constantOr(FillColor::defaultValue()),
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                0,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillPropsUniformBuffer, &paramsUBO);
        }
    };

    const auto UpdateFillOutlineUniformBuffers = [&]() {
        if (fillOutlineUniformBufferUpdated) return;
        fillOutlineUniformBufferUpdated = true;

        if (!fillOutlinePropsUniformBuffer || propertiesUpdated) {
            const FillOutlineEvaluatedPropsUBO paramsUBO = {
                /* .outline_color = */ evaluated.get<FillOutlineColor>().constantOr(FillOutlineColor::defaultValue()),
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                0,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillOutlinePropsUniformBuffer, &paramsUBO);
        }
    };

    const auto UpdateFillPatternUniformBuffers = [&]() {
        if (fillPatternUniformBufferUpdated) return;
        fillPatternUniformBufferUpdated = true;

        if (!fillPatternPropsUniformBuffer || propertiesUpdated) {
            const FillPatternEvaluatedPropsUBO paramsUBO = {
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                /* .fade = */ crossfade.t,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillPatternPropsUniformBuffer, &paramsUBO);
        }
    };

    const auto UpdateFillOutlinePatternUniformBuffers = [&]() {
        if (fillOutlinePatternUniformBufferUpdated) return;
        fillOutlinePatternUniformBufferUpdated = true;

        if (!fillOutlinePatternPropsUniformBuffer || propertiesUpdated) {
            const FillOutlinePatternEvaluatedPropsUBO paramsUBO = {
                /* .opacity = */ evaluated.get<FillOpacity>().constantOr(FillOpacity::defaultValue()),
                /* .fade = */ crossfade.t,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillOutlinePatternPropsUniformBuffer, &paramsUBO);
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
        const auto matrix = getTileMatrix(tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits);

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
        if (uniforms.get(idFillInterpolateUBOName)) {
            UpdateFillUniformBuffers();

            uniforms.addOrReplace(idFillEvaluatedPropsUBOName, fillPropsUniformBuffer);

            const FillDrawableUBO drawableUBO = {/*.matrix=*/util::cast<float>(matrix)};
            uniforms.createOrUpdate(idFillDrawableUBOName, &drawableUBO, context);
        } else if (uniforms.get(idFillOutlineInterpolateUBOName)) {
            UpdateFillOutlineUniformBuffers();

            uniforms.addOrReplace(idFillOutlineEvaluatedPropsUBOName, fillOutlinePropsUniformBuffer);

            const FillOutlineDrawableUBO drawableUBO = {
                /*.matrix=*/util::cast<float>(matrix),
                /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
                /* pad1 */ 0,
                /* pad2 */ 0};
            uniforms.createOrUpdate(idFillOutlineDrawableUBOName, &drawableUBO, context);
        } else if (uniforms.get(idFillPatternInterpolateUBOName)) {
            UpdateFillPatternUniformBuffers();

            uniforms.addOrReplace(idFillPatternEvaluatedPropsUBOName, fillPatternPropsUniformBuffer);

            const FillPatternDrawableUBO drawableUBO = {
                /*.matrix=*/util::cast<float>(matrix),
                /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
                /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
                0,
                0,
            };
            uniforms.createOrUpdate(idFillPatternDrawableUBOName, &drawableUBO, context);
        } else if (uniforms.get(idFillOutlinePatternInterpolateUBOName)) {
            UpdateFillOutlinePatternUniformBuffers();

            uniforms.addOrReplace(idFillOutlinePatternEvaluatedPropsUBOName, fillOutlinePatternPropsUniformBuffer);

            const FillOutlinePatternDrawableUBO drawableUBO = {
                /*.matrix=*/util::cast<float>(matrix),
                /*.scale=*/{pixelRatio, tileRatio, crossfade.fromScale, crossfade.toScale},
                /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
                /*.pixel_coord_upper=*/{static_cast<float>(pixelX >> 16), static_cast<float>(pixelY >> 16)},
                /*.pixel_coord_lower=*/{static_cast<float>(pixelX & 0xFFFF), static_cast<float>(pixelY & 0xFFFF)},
                /*.texsize=*/{static_cast<float>(textureSize.width), static_cast<float>(textureSize.height)},
            };
            uniforms.createOrUpdate(idFillOutlinePatternDrawableUBOName, &drawableUBO, context);
        }
    });

    propertiesUpdated = false;
}

} // namespace mbgl
