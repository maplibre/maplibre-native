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

static const StringIdentity idFillDrawableUBOName = StringIndexer::get("FillDrawableUBO");
static const StringIdentity idFillDrawablePropsUBOName = StringIndexer::get("FillDrawablePropsUBO");
static const StringIdentity idFillEvaluatedPropsUBOName = StringIndexer::get("FillEvaluatedPropsUBO");
static const StringIdentity idFillPermutationUBOName = StringIndexer::get("FillPermutationUBO");

const StringIdentity FillLayerTweaker::idFillTilePropsUBOName = StringIndexer::get("FillDrawableTilePropsUBO");
const StringIdentity FillLayerTweaker::idFillInterpolateUBOName = StringIndexer::get("FillInterpolateUBO");
const StringIdentity FillLayerTweaker::idFillOutlineInterpolateUBOName = StringIndexer::get(
    "FillOutlineInterpolateUBO");

static const StringIdentity idFillOutlineDrawableUBOName = StringIndexer::get("FillOutlineDrawableUBO");
static const StringIdentity idFillOutlineEvaluatedPropsUBOName = StringIndexer::get("FillOutlineEvaluatedPropsUBO");
static const StringIdentity idFillOutlinePermutationUBOName = StringIndexer::get("FillOutlinePermutationUBO");

static const StringIdentity idFillOutlineInterpolateUBOName = StringIndexer::get("FillOutlineInterpolateUBO");

static const StringIdentity idFillPatternDrawableUBOName = StringIndexer::get("FillPatternDrawableUBO");
static const StringIdentity idFillPatternPermutationUBOName = StringIndexer::get("FillPatternPermutationUBO");
static const StringIdentity idFillPatternInterpolateUBOName = StringIndexer::get("FillPatternInterpolateUBO");
static const StringIdentity idFillPatternEvaluatedPropsUBOName = StringIndexer::get("FillPatternEvaluatedPropsUBO");
static const StringIdentity idFillPatternTilePropsUBOName = StringIndexer::get("FillPatternTilePropsUBO");

static const StringIdentity idFillOutlinePatternDrawableUBOName = StringIndexer::get("FillOutlinePatternDrawableUBO");
static const StringIdentity idFillOutlinePatternPermutationUBOName = StringIndexer::get(
    "FillOutlinePatternPermutationUBO");
static const StringIdentity idFillOutlinePatternInterpolateUBOName = StringIndexer::get(
    "FillOutlinePatternInterpolateUBO");
static const StringIdentity idFillOutlinePatternEvaluatedPropsUBOName = StringIndexer::get(
    "FillOutlinePatternEvaluatedPropsUBO");
static const StringIdentity idFillOutlinePatternTilePropsUBOName = StringIndexer::get("FillOutlinePatternTilePropsUBO");

static const StringIdentity idExpressionInputsUBOName = StringIndexer::get("ExpressionInputsUBO");

static const StringIdentity idTexImageName = StringIndexer::get("u_image");
using namespace shaders;

void FillLayerTweaker::execute(LayerGroupBase& layerGroup,
                               const PaintParameters& parameters) {
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

#if MLN_RENDER_BACKEND_METAL
        if (propertiesChanged || !fillPermutationUniformBuffer) {
            const FillPermutationUBO permutationUBO = {
                /* .color = */ {/*.source=*/getAttributeSource<BuiltIn::FillShader>(1), /*.expression=*/{}},
                /* .opacity = */ {/*.source=*/getAttributeSource<BuiltIn::FillShader>(2), /*.expression=*/{}},
                /* .overdrawInspector = */ overdrawInspector,
                0,
                0,
                0,
                0,
                0,
                0,
            };

            context.emplaceOrUpdateUniformBuffer(fillPermutationUniformBuffer, &permutationUBO);
        }
#endif

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

#if MLN_RENDER_BACKEND_METAL
        if (propertiesChanged || !fillOutlinePermutationUniformBuffer) {
            const FillOutlinePermutationUBO permutationUBO = {
                /* .outline_color = */ {/*.source=*/getAttributeSource<BuiltIn::FillOutlineShader>(1),
                                        /*.expression=*/{}},
                /* .opacity = */ {/*.source=*/getAttributeSource<BuiltIn::FillOutlineShader>(2), /*.expression=*/{}},
                /* .overdrawInspector = */ overdrawInspector,
                0,
                0,
                0,
                0,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillOutlinePermutationUniformBuffer, &permutationUBO);
        }
#endif

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

#if MLN_RENDER_BACKEND_METAL
        if (propertiesChanged || !fillPatternPermutationUniformBuffer) {
            const FillPatternPermutationUBO permutationUBO = {
                /* .pattern_from = */ {/*.source=*/getAttributeSource<BuiltIn::FillPatternShader>(1),
                                       /*.expression=*/{}},
                /* .pattern_to = */
                {/*.source=*/getAttributeSource<BuiltIn::FillPatternShader>(2), /*.expression=*/{}},
                /* .opacity = */
                {/*.source=*/getAttributeSource<BuiltIn::FillPatternShader>(3), /*.expression=*/{}},
                /* .overdrawInspector = */ overdrawInspector,
                0,
                0,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillPatternPermutationUniformBuffer, &permutationUBO);
        }
#endif

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

#if MLN_RENDER_BACKEND_METAL
        if (propertiesChanged || !fillOutlinePatternPermutationUniformBuffer) {
            const FillOutlinePatternPermutationUBO permutationUBO = {
                /* .pattern_from = */ {/*.source=*/getAttributeSource<BuiltIn::FillOutlinePatternShader>(1),
                                       /*.expression=*/{}},
                /* .pattern_to = */
                {/*.source=*/getAttributeSource<BuiltIn::FillOutlinePatternShader>(2), /*.expression=*/{}},
                /* .opacity = */
                {/*.source=*/getAttributeSource<BuiltIn::FillOutlinePatternShader>(3), /*.expression=*/{}},
                /* .overdrawInspector = */ overdrawInspector,
                0,
                0,
                0,
                0,
            };
            context.emplaceOrUpdateUniformBuffer(fillOutlinePatternPermutationUniformBuffer, &permutationUBO);
        }
#endif

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

#if MLN_RENDER_BACKEND_METAL
    const auto zoom = parameters.state.getZoom();
    const auto expressionUBO = buildExpressionUBO(zoom, parameters.frameCount);
    context.emplaceOrUpdateUniformBuffer(expressionUniformBuffer, &expressionUBO);
#endif

    const auto& translation = evaluated.get<FillTranslate>();
    const auto anchor = evaluated.get<FillTranslateAnchor>();

    const auto renderableSize = parameters.backend.getDefaultRenderable().getSize();
    const auto intZoom = parameters.state.getIntegerZoom();
    const auto pixelRatio = parameters.pixelRatio;

    layerGroup.visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(
            tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits);

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

#if MLN_RENDER_BACKEND_METAL
            uniforms.addOrReplace(idFillPermutationUBOName, fillPermutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
        } else if (uniforms.get(idFillOutlineInterpolateUBOName)) {
            UpdateFillOutlineUniformBuffers();

            uniforms.addOrReplace(idFillOutlineEvaluatedPropsUBOName, fillOutlinePropsUniformBuffer);

            const FillOutlineDrawableUBO drawableUBO = {
                /*.matrix=*/util::cast<float>(matrix),
                /*.world=*/{(float)renderableSize.width, (float)renderableSize.height},
                /* pad1 */ 0,
                /* pad2 */ 0};
            uniforms.createOrUpdate(idFillOutlineDrawableUBOName, &drawableUBO, context);

#if MLN_RENDER_BACKEND_METAL
            uniforms.addOrReplace(idFillOutlinePermutationUBOName, fillOutlinePermutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
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

#if MLN_RENDER_BACKEND_METAL
            uniforms.addOrReplace(idFillPatternPermutationUBOName, fillPatternPermutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
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

#if MLN_RENDER_BACKEND_METAL
            uniforms.addOrReplace(idFillOutlinePatternPermutationUBOName, fillOutlinePatternPermutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
        }

#if MLN_RENDER_BACKEND_METAL
        uniforms.addOrReplace(idExpressionInputsUBOName, expressionUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
    });

    propertiesChanged = false;
    propertiesUpdated = false;
}

} // namespace mbgl
