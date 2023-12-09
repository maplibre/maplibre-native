#include <mbgl/renderer/layers/circle_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/style/layers/circle_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/string_indexer.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/circle.hpp>
#endif

#include <cstring>

namespace mbgl {

using namespace style;
using namespace shaders;

static const StringIdentity idCircleDrawableUBOName = stringIndexer().get("CircleDrawableUBO");
static const StringIdentity idCirclePaintParamsUBOName = stringIndexer().get("CirclePaintParamsUBO");
static const StringIdentity idCircleEvaluatedPropsUBOName = stringIndexer().get("CircleEvaluatedPropsUBO");

void CircleLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto& evaluated = static_cast<const CircleLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    // Updated every frame, but shared across drawables
    const CirclePaintParamsUBO paintParamsUBO = {
        /* .camera_to_center_distance = */ parameters.state.getCameraToCenterDistance(),
        /* .padding = */ 0,
        0,
        0};

    if (!paintParamsUniformBuffer) {
        paintParamsUniformBuffer = context.createUniformBuffer(&paintParamsUBO, sizeof(paintParamsUBO));
    } else {
        paintParamsUniformBuffer->update(&paintParamsUBO, sizeof(CirclePaintParamsUBO));
    }

    const auto zoom = parameters.state.getZoom();
    const bool pitchWithMap = evaluated.get<CirclePitchAlignment>() == AlignmentType::Map;
    const bool scaleWithMap = evaluated.get<CirclePitchScale>() == CirclePitchScaleType::Map;

    // Updated only with evaluated properties
    if (!evaluatedPropsUniformBuffer) {
        const CircleEvaluatedPropsUBO evaluatedPropsUBO = {
            /* .color = */ constOrDefault<CircleColor>(evaluated),
            /* .stroke_color = */ constOrDefault<CircleStrokeColor>(evaluated),
            /* .radius = */ constOrDefault<CircleRadius>(evaluated),
            /* .blur = */ constOrDefault<CircleBlur>(evaluated),
            /* .opacity = */ constOrDefault<CircleOpacity>(evaluated),
            /* .stroke_width = */ constOrDefault<CircleStrokeWidth>(evaluated),
            /* .stroke_opacity = */ constOrDefault<CircleStrokeOpacity>(evaluated),
            /* .scale_with_map = */ scaleWithMap,
            /* .pitch_with_map = */ pitchWithMap,
            /* .padding = */ 0};
        evaluatedPropsUniformBuffer = context.createUniformBuffer(&evaluatedPropsUBO, sizeof(evaluatedPropsUBO));
    }

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        assert(drawable.getTileID() || !"Circles only render with tiles");
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.addOrReplace(idCirclePaintParamsUBOName, paintParamsUniformBuffer);
        uniforms.addOrReplace(idCircleEvaluatedPropsUBOName, evaluatedPropsUniformBuffer);

        const auto& translation = evaluated.get<CircleTranslate>();
        const auto anchor = evaluated.get<CircleTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits);

        const auto pixelsToTileUnits = tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom));
        const auto extrudeScale = pitchWithMap ? std::array<float, 2>{pixelsToTileUnits, pixelsToTileUnits}
                                               : parameters.pixelsToGLUnits;

        // Updated for each drawable on each frame
        const CircleDrawableUBO drawableUBO = {/* .matrix = */ util::cast<float>(matrix),
                                               /* .extrude_scale = */ extrudeScale,
                                               /* .padding = */ 0};

        uniforms.createOrUpdate(idCircleDrawableUBOName, &drawableUBO, context);
    });
}

} // namespace mbgl
