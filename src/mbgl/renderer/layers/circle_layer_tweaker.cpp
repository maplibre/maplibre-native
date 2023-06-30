#include <mbgl/renderer/layers/circle_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/style/layers/circle_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) CircleDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> extrude_scale;
    std::array<float, 2> padding;
};
static_assert(sizeof(CircleDrawableUBO) % 16 == 0);

struct alignas(16) CirclePaintParamsUBO {
    float camera_to_center_distance;
    float device_pixel_ratio;
    std::array<float, 2> padding;
};
static_assert(sizeof(CirclePaintParamsUBO) % 16 == 0);

struct alignas(16) CircleEvaluatedPropsUBO {
    Color color;
    Color stroke_color;
    float radius;
    float blur;
    float opacity;
    float stroke_width;
    float stroke_opacity;
    int scale_with_map;
    int pitch_with_map;
    float padding;
};
static_assert(sizeof(CircleEvaluatedPropsUBO) % 16 == 0);

static constexpr std::string_view CircleDrawableUBOName = "CircleDrawableUBO";
static constexpr std::string_view CirclePaintParamsUBOName = "CirclePaintParamsUBO";
static constexpr std::string_view CircleEvaluatedPropsUBOName = "CircleEvaluatedPropsUBO";

void CircleLayerTweaker::execute(LayerGroupBase& layerGroup,
                                 const RenderTree& renderTree,
                                 const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const CircleLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    const CirclePaintParamsUBO paintParamsUBO = {
        /* .camera_to_center_distance = */ parameters.state.getCameraToCenterDistance(),
        /* .device_pixel_ratio = */ parameters.pixelRatio,
        /* .padding = */ {0}};

    if (!paintParamsUniformBuffer) {
        paintParamsUniformBuffer = parameters.context.createUniformBuffer(&paintParamsUBO, sizeof(paintParamsUBO));
    } else {
        paintParamsUniformBuffer->update(&paintParamsUBO, sizeof(CirclePaintParamsUBO));
    }

    const bool pitchWithMap = evaluated.get<CirclePitchAlignment>() == AlignmentType::Map;

    if (!evaluatedPropsUniformBuffer) {
        const CircleEvaluatedPropsUBO evaluatedPropsUBO = {
            /* .color = */ evaluated.get<CircleColor>().constantOr(CircleColor::defaultValue()),
            /* .stroke_color = */ evaluated.get<CircleStrokeColor>().constantOr(CircleStrokeColor::defaultValue()),
            /* .radius = */ evaluated.get<CircleRadius>().constantOr(CircleRadius::defaultValue()),
            /* .blur = */ evaluated.get<CircleBlur>().constantOr(CircleBlur::defaultValue()),
            /* .opacity = */ evaluated.get<CircleOpacity>().constantOr(CircleOpacity::defaultValue()),
            /* .stroke_width = */ evaluated.get<CircleStrokeWidth>().constantOr(CircleStrokeWidth::defaultValue()),
            /* .stroke_opacity = */
            evaluated.get<CircleStrokeOpacity>().constantOr(CircleStrokeOpacity::defaultValue()),
            /* .scale_with_map = */ evaluated.get<CirclePitchScale>() == CirclePitchScaleType::Map,
            /* .pitch_with_map = */ pitchWithMap,
            /* .padding = */ 0};
        evaluatedPropsUniformBuffer = parameters.context.createUniformBuffer(&evaluatedPropsUBO,
                                                                             sizeof(evaluatedPropsUBO));
    }

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        drawable.mutableUniformBuffers().addOrReplace(CirclePaintParamsUBOName, paintParamsUniformBuffer);
        drawable.mutableUniformBuffers().addOrReplace(CircleEvaluatedPropsUBOName, evaluatedPropsUniformBuffer);

        if (!drawable.getTileID()) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto& translation = evaluated.get<CircleTranslate>();
        const auto anchor = evaluated.get<CircleTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, nearClipped, inViewportPixelUnits);

        const auto pixelsToTileUnits = tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()));

        CircleDrawableUBO drawableUBO = {
            /* .matrix = */ util::cast<float>(matrix),
            /* .extrude_scale = */
            pitchWithMap ? std::array<float, 2>{{pixelsToTileUnits}} : parameters.pixelsToGLUnits,
            /* .padding = */ {0}};

        drawable.mutableUniformBuffers().createOrUpdate(CircleDrawableUBOName, &drawableUBO, parameters.context);
    });
}

} // namespace mbgl
