#include <mbgl/renderer/layers/circle_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/buckets/circle_bucket.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/style/layers/circle_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/circle.hpp>
#endif

#include <cstring>
#include <bit>

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {
constexpr uint32_t CircleExpressionColor = 1u << 0u;
constexpr uint32_t CircleExpressionRadius = 1u << 1u;
constexpr uint32_t CircleExpressionBlur = 1u << 2u;
constexpr uint32_t CircleExpressionOpacity = 1u << 3u;
constexpr uint32_t CircleExpressionStrokeColor = 1u << 4u;
constexpr uint32_t CircleExpressionStrokeWidth = 1u << 5u;
constexpr uint32_t CircleExpressionStrokeOpacity = 1u << 6u;
} // namespace

void CircleLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    auto& context = parameters.context;
    const auto& evaluated = static_cast<const CircleLayerProperties&>(*evaluatedProperties).evaluated;

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    const auto zoom = static_cast<float>(parameters.state.getZoom());
    const bool pitchWithMap = evaluated.get<CirclePitchAlignment>() == AlignmentType::Map;
    const bool scaleWithMap = evaluated.get<CirclePitchScale>() == CirclePitchScaleType::Map;

    // Updated only with evaluated properties
    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
        uint32_t expressionMask = 0;
        if (!evaluated.get<CircleColor>().isConstant()) {
            expressionMask |= CircleExpressionColor;
        }
        if (!evaluated.get<CircleRadius>().isConstant()) {
            expressionMask |= CircleExpressionRadius;
        }
        if (!evaluated.get<CircleBlur>().isConstant()) {
            expressionMask |= CircleExpressionBlur;
        }
        if (!evaluated.get<CircleOpacity>().isConstant()) {
            expressionMask |= CircleExpressionOpacity;
        }
        if (!evaluated.get<CircleStrokeColor>().isConstant()) {
            expressionMask |= CircleExpressionStrokeColor;
        }
        if (!evaluated.get<CircleStrokeWidth>().isConstant()) {
            expressionMask |= CircleExpressionStrokeWidth;
        }
        if (!evaluated.get<CircleStrokeOpacity>().isConstant()) {
            expressionMask |= CircleExpressionStrokeOpacity;
        }

        const CircleEvaluatedPropsUBO evaluatedPropsUBO = {
            .color = constOrDefault<CircleColor>(evaluated),
            .stroke_color = constOrDefault<CircleStrokeColor>(evaluated),
            .radius = constOrDefault<CircleRadius>(evaluated),
            .blur = constOrDefault<CircleBlur>(evaluated),
            .opacity = constOrDefault<CircleOpacity>(evaluated),
            .stroke_width = constOrDefault<CircleStrokeWidth>(evaluated),
            .stroke_opacity = constOrDefault<CircleStrokeOpacity>(evaluated),
            .scale_with_map = scaleWithMap,
            .pitch_with_map = pitchWithMap,
            .pad1 = std::bit_cast<float>(expressionMask)};
        context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &evaluatedPropsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idCircleEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<CircleDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        assert(drawable.getTileID() || !"Circles only render with tiles");
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        auto* binders = static_cast<CircleBinders*>(drawable.getBinders());
        if (!binders) {
            assert(false);
            return;
        }

        const auto& translation = evaluated.get<CircleTranslate>();
        const auto anchor = evaluated.get<CircleTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(
            tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits, drawable);

        const auto pixelsToTileUnits = tileID.pixelsToTileUnits(1.0f, zoom);
        const auto extrudeScale = pitchWithMap ? std::array<float, 2>{pixelsToTileUnits, pixelsToTileUnits}
                                               : parameters.pixelsToGLUnits;

#if MLN_UBO_CONSOLIDATION
        drawableUBOVector[i] = {
#else
        const CircleDrawableUBO drawableUBO = {
#endif

            .matrix = util::cast<float>(matrix),
            .extrude_scale = extrudeScale,

            .color_t = std::get<0>(binders->get<CircleColor>()->interpolationFactor(zoom)),
            .radius_t = std::get<0>(binders->get<CircleRadius>()->interpolationFactor(zoom)),
            .blur_t = std::get<0>(binders->get<CircleBlur>()->interpolationFactor(zoom)),
            .opacity_t = std::get<0>(binders->get<CircleOpacity>()->interpolationFactor(zoom)),
            .stroke_color_t = std::get<0>(binders->get<CircleStrokeColor>()->interpolationFactor(zoom)),
            .stroke_width_t = std::get<0>(binders->get<CircleStrokeWidth>()->interpolationFactor(zoom)),
            .stroke_opacity_t = std::get<0>(binders->get<CircleStrokeOpacity>()->interpolationFactor(zoom)),
            .pad1 = 0,
            .pad2 = 0,
            .pad3 = 0
        };
#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#else
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idCircleDrawableUBO, &drawableUBO, context);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const size_t drawableUBOVectorSize = sizeof(CircleDrawableUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    layerUniforms.set(idCircleDrawableUBO, drawableUniformBuffer);
#endif
}

} // namespace mbgl
