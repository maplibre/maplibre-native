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

struct alignas(16) CircleInterpolateUBO {
    float color_t;
    float radius_t;
    float blur_t;
    float opacity_t;
    float stroke_color_t;
    float stroke_width_t;
    float stroke_opacity_t;
    float padding;
};
static_assert(sizeof(CircleInterpolateUBO) % 16 == 0);

void CircleLayerTweaker::execute(LayerGroup& layerGroup,
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

    CirclePaintParamsUBO paintParamsUBO;
    paintParamsUBO.camera_to_center_distance = parameters.state.getCameraToCenterDistance();
    paintParamsUBO.device_pixel_ratio = parameters.pixelRatio;

    if (!paintParamsUniformBuffer) {
        paintParamsUniformBuffer = parameters.context.createUniformBuffer(&paintParamsUBO, sizeof(paintParamsUBO));
    } else {
        paintParamsUniformBuffer->update(&paintParamsUBO, sizeof(CirclePaintParamsUBO));
    }

    const bool pitchWithMap = evaluated.get<CirclePitchAlignment>() == AlignmentType::Map;

    if (!evaluatedPropsUniformBuffer) {
        CircleEvaluatedPropsUBO evaluatedPropsUBO;
        evaluatedPropsUBO.color = evaluated.get<CircleColor>().constantOr(Color());
        evaluatedPropsUBO.radius = evaluated.get<CircleRadius>().constantOr(0);
        evaluatedPropsUBO.blur = evaluated.get<CircleBlur>().constantOr(0);
        evaluatedPropsUBO.opacity = evaluated.get<CircleOpacity>().constantOr(0);
        evaluatedPropsUBO.stroke_color = evaluated.get<CircleStrokeColor>().constantOr(Color());
        evaluatedPropsUBO.stroke_width = evaluated.get<CircleStrokeWidth>().constantOr(0);
        evaluatedPropsUBO.stroke_opacity = evaluated.get<CircleStrokeOpacity>().constantOr(0);
        evaluatedPropsUBO.scale_with_map = evaluated.get<CirclePitchScale>() == CirclePitchScaleType::Map;
        evaluatedPropsUBO.pitch_with_map = pitchWithMap;
        evaluatedPropsUniformBuffer = parameters.context.createUniformBuffer(&evaluatedPropsUBO,
                                                                             sizeof(evaluatedPropsUBO));
    }

    CircleInterpolateUBO interpolateUBO;
    interpolateUBO.color_t = 0;
    interpolateUBO.radius_t = 0;
    interpolateUBO.blur_t = 0;
    interpolateUBO.opacity_t = 0;
    interpolateUBO.stroke_color_t = 0;
    interpolateUBO.stroke_width_t = 0;
    interpolateUBO.stroke_opacity_t = 0;

    if (!interpolateUniformBuffer) {
        interpolateUniformBuffer = parameters.context.createUniformBuffer(&interpolateUBO, sizeof(interpolateUBO));
    } else {
        interpolateUniformBuffer->update(&interpolateUBO, sizeof(CircleInterpolateUBO));
    }

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        drawable.mutableUniformBuffers().addOrReplace("CirclePaintParamsUBO", paintParamsUniformBuffer);
        drawable.mutableUniformBuffers().addOrReplace("CircleEvaluatedPropsUBO", evaluatedPropsUniformBuffer);
        drawable.mutableUniformBuffers().addOrReplace("CircleInterpolateUBO", interpolateUniformBuffer);

        if (!drawable.getTileID()) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto& translation = evaluated.get<CircleTranslate>();
        const auto anchor = evaluated.get<CircleTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, inViewportPixelUnits);

        CircleDrawableUBO drawableUBO;
        drawableUBO.matrix = util::cast<float>(matrix);
        drawableUBO.extrude_scale =
            pitchWithMap
                ? std::array<float, 2>{{tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom())),
                                        tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()))}}
                : parameters.pixelsToGLUnits;

        if (auto& ubo = drawable.mutableUniformBuffers().get("CircleDrawableUBO")) {
            ubo->update(&drawableUBO, sizeof(drawableUBO));
        } else {
            auto drawableUniformBuffer = parameters.context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
            drawable.mutableUniformBuffers().addOrReplace("CircleDrawableUBO", drawableUniformBuffer);
        }
    });
}

} // namespace mbgl
