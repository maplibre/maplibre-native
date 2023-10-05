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
#include <mbgl/util/logging.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/string_indexer.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/style/expression/parsing_context.hpp>
#include <mbgl/shaders/mtl/fill.hpp>
#endif

namespace mbgl {

using namespace style;

static const StringIdentity idFillDrawableUBOName = stringIndexer().get("FillDrawableUBO");
static const StringIdentity idFillDrawablePropsUBOName = stringIndexer().get("FillDrawablePropsUBO");
static const StringIdentity idFillEvaluatedPropsUBOName = stringIndexer().get("FillEvaluatedPropsUBO");
static const StringIdentity idFillPermutationUBOName = stringIndexer().get("FillPermutationUBO");

const StringIdentity FillLayerTweaker::idFillTilePropsUBOName = stringIndexer().get("FillDrawableTilePropsUBO");
const StringIdentity FillLayerTweaker::idFillInterpolateUBOName = stringIndexer().get("FillInterpolateUBO");
const StringIdentity FillLayerTweaker::idFillOutlineInterpolateUBOName = stringIndexer().get(
    "FillOutlineInterpolateUBO");

static const StringIdentity idFillOutlineDrawableUBOName = stringIndexer().get("FillOutlineDrawableUBO");
static const StringIdentity idFillOutlineEvaluatedPropsUBOName = stringIndexer().get("FillOutlineEvaluatedPropsUBO");
static const StringIdentity idFillOutlinePermutationUBOName = stringIndexer().get("FillOutlinePermutationUBO");

static const StringIdentity idFillOutlineInterpolateUBOName = stringIndexer().get("FillOutlineInterpolateUBO");

static const StringIdentity idFillPatternDrawableUBOName = stringIndexer().get("FillPatternDrawableUBO");
static const StringIdentity idFillPatternPermutationUBOName = stringIndexer().get("FillPatternPermutationUBO");
static const StringIdentity idFillPatternInterpolateUBOName = stringIndexer().get("FillPatternInterpolateUBO");
static const StringIdentity idFillPatternEvaluatedPropsUBOName = stringIndexer().get("FillPatternEvaluatedPropsUBO");
static const StringIdentity idFillPatternTilePropsUBOName = stringIndexer().get("FillPatternTilePropsUBO");

static const StringIdentity idFillOutlinePatternDrawableUBOName = stringIndexer().get("FillOutlinePatternDrawableUBO");
static const StringIdentity idFillOutlinePatternPermutationUBOName = stringIndexer().get(
    "FillOutlinePatternPermutationUBO");
static const StringIdentity idFillOutlinePatternInterpolateUBOName = stringIndexer().get(
    "FillOutlinePatternInterpolateUBO");
static const StringIdentity idFillOutlinePatternEvaluatedPropsUBOName = stringIndexer().get(
    "FillOutlinePatternEvaluatedPropsUBO");
static const StringIdentity idFillOutlinePatternTilePropsUBOName = stringIndexer().get(
    "FillOutlinePatternTilePropsUBO");

static const StringIdentity idExpressionInputsUBOName = stringIndexer().get("ExpressionInputsUBO");

static const StringIdentity idTexImageName = stringIndexer().get("u_image");
using namespace shaders;

bool populateExpression(const Transitioning<PropertyValue<float>>& transitionProperty,
                        shaders::Expression& shaderExpr) {
    if (transitionProperty.isUndefined()) {
        // we need two expressions per property to support transitions
        return false;
    }

    if (transitionProperty.hasTransition()) {
        // We need two expressions per property to support transitions.
        // But for now, we won't run this again, so just ignore it.
    }

    const PropertyValue<float>& property = transitionProperty.getValue();
    if (!property.isExpression()) {
        return false;
    }

    // We're only interested in expressions that are "feature-constant" (meaning it doesn't
    // depend on the data, a.k.a., not "data-driven") and "runtime-constant," which seems to
    // be equivalent to "not image-based".
    const style::PropertyExpression<float>& propertyExpression = property.asExpression();
    const expression::Expression& expression = propertyExpression.getExpression();
    if (!expression.getType().is<style::expression::type::NumberType>() || !propertyExpression.isFeatureConstant() ||
        !propertyExpression.isRuntimeConstant()) {
        return false;
    }

    memset(&shaderExpr, 0, sizeof(shaderExpr));
    shaderExpr.function = ExpressionFunction::Linear;
    shaderExpr.useIntegerZoom = propertyExpression.useIntegerZoom;

    bool failed = false;
    const auto populateStops = [&](double zoom, const expression::Expression& stopExpr) {
        if (!failed && shaderExpr.stopCount + 1 < shaders::ExprMaxStops &&
            stopExpr.getType() == style::expression::type::Number && isConstant(stopExpr)) {
            const style::expression::EvaluationResult result = stopExpr.evaluate(
                style::expression::EvaluationContext{});
            if (result && result->is<double>()) {
                const auto stopValue = result->get<double>();
                shaderExpr.zooms[shaderExpr.stopCount] = static_cast<float>(zoom);
                shaderExpr.values[shaderExpr.stopCount] = static_cast<float>(stopValue);
                shaderExpr.stopCount += 1;
                return;
            }
        }
        failed = true;
    };

    if (const auto steps = propertyExpression.getZoomSteps()) {
        steps->eachStop(populateStops);
    } else if (auto interp = propertyExpression.getZoomInterplation()) {
        interp->eachStop(populateStops);
    }

    return (!failed && shaderExpr.stopCount > 0);
}

bool populateExpression(const Transitioning<PropertyValue<Color>>& transitionProperty,
                        shaders::ColorExpression& shaderExpr) {
    if (transitionProperty.isUndefined() || transitionProperty.hasTransition()) {
        // we need two expressions per property to support transitions
        return false;
    }

    const PropertyValue<Color>& property = transitionProperty.getValue();
    if (!property.isExpression()) {
        return false;
    }

    // We're only interested in expressions that are "feature-constant" (meaning it doesn't
    // depend on the data, a.k.a., not "data-driven") and "runtime-constant," which seems to
    // be equivalent to "not image-based".
    const style::PropertyExpression<Color>& propertyExpression = property.asExpression();
    const expression::Expression& expression = propertyExpression.getExpression();
    if (!expression.getType().is<style::expression::type::ColorType>() || !propertyExpression.isFeatureConstant() ||
        !propertyExpression.isRuntimeConstant()) {
        return false;
    }

    memset(&shaderExpr, 0, sizeof(shaderExpr));
    shaderExpr.function = ExpressionFunction::Linear;
    shaderExpr.useIntegerZoom = propertyExpression.useIntegerZoom;

    bool failed = false;
    const auto populateStops = [&](double zoom, const expression::Expression& stopExpr) {
        if (!failed && shaderExpr.stopCount + 1 < shaders::ExprMaxStops &&
            stopExpr.getType() == style::expression::type::Color && isConstant(stopExpr)) {
            const style::expression::EvaluationResult result = stopExpr.evaluate(
                style::expression::EvaluationContext{});
            if (result && result->is<Color>()) {
                const auto& stopColor = result->get<Color>();
                shaderExpr.zooms[shaderExpr.stopCount] = static_cast<float>(zoom);
                shaderExpr.values[shaderExpr.stopCount][0] = stopColor.r;
                shaderExpr.values[shaderExpr.stopCount][1] = stopColor.g;
                shaderExpr.values[shaderExpr.stopCount][2] = stopColor.b;
                shaderExpr.values[shaderExpr.stopCount][3] = stopColor.a;
                shaderExpr.stopCount += 1;
                return;
            }
        }
        failed = true;
    };

    if (const auto steps = propertyExpression.getZoomSteps()) {
        steps->eachStop(populateStops);
    } else if (auto interp = propertyExpression.getZoomInterplation()) {
        interp->eachStop(populateStops);
    }

    return (!failed && shaderExpr.stopCount > 0);
}

void FillLayerTweaker::buildAttributeExpressions(const style::FillPaintProperties::Unevaluated& unevaluated) {
    shaders::ColorExpression colorExpr;
    if (populateExpression(unevaluated.get<style::FillColor>(), colorExpr)) {
        fillColorExpr = colorExpr;
    }

    shaders::Expression floatExpr;
    if (populateExpression(unevaluated.get<style::FillOpacity>(), floatExpr)) {
        opacityExpr = floatExpr;
    }
}

shaders::ExpressionAttribute FillLayerTweaker::getAttribute(const StringIdentity attrNameID,
                                                            const std::optional<shaders::Expression>& attr) {
    if (attr) {
        // from uniforms or attribute arrays
        return {
            /*.expression=*/*attr,
            /*.source=*/AttributeSource::Computed,
        };
    } else {
        // from uniforms or attribute arrays
        return {
            /*.expression=*/{},
            /*.source=*/getAttributeSource(attrNameID),
        };
    }
}

shaders::ColorAttribute FillLayerTweaker::getAttribute(const StringIdentity attrNameID,
                                                       const std::optional<shaders::ColorExpression>& attr) {
    if (attr) {
        // from uniforms or attribute arrays
        return {/*.expression=*/*attr,
                /*.source=*/AttributeSource::Computed,
                /*.pad=*/0,
                0,
                0};
    } else {
        // from uniforms or attribute arrays
        return {/*.expression=*/{},
                /*.source=*/getAttributeSource(attrNameID),
                /*.pad=*/0,
                0,
                0};
    }
};

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

#if MLN_RENDER_BACKEND_METAL
        if (permutationUpdated || !fillPermutationUniformBuffer) {
            using ShaderClass = shaders::ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>;
            const auto colorNameID = ShaderClass::attributes[ShaderClass::a_color_index].nameID;
            const auto opacityNameID = ShaderClass::attributes[ShaderClass::a_opacity_index].nameID;

            const FillPermutationUBO permutationUBO = {
                /* .color = */ getAttribute(colorNameID, fillColorExpr),
                /* .opacity = */ getAttribute(opacityNameID, opacityExpr),
                /* .overdrawInspector = */ overdrawInspector,
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
        if (permutationUpdated || !fillOutlinePermutationUniformBuffer) {
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
        if (permutationUpdated || !fillPatternPermutationUniformBuffer) {
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
        if (permutationUpdated || !fillOutlinePatternPermutationUniformBuffer) {
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

    permutationUpdated = false;
    propertiesUpdated = false;
}

} // namespace mbgl
