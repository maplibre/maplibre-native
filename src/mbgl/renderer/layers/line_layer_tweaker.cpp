#include <mbgl/renderer/layers/line_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) LineDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> units_to_pixels;
    float ratio;
    float device_pixel_ratio;
};
static_assert(sizeof(LineDrawableUBO) % 16 == 0);

void LineLayerTweaker::execute(LayerGroup& /*layerGroup*/, const PaintParameters& /*parameters*/) {
    // const auto& evaluated = static_cast<const CircleLayerProperties&>(*evaluatedProperties).evaluated;

    // CirclePaintParamsUBO paintParamsUBO;
    // paintParamsUBO.camera_to_center_distance = parameters.state.getCameraToCenterDistance();
    // paintParamsUBO.device_pixel_ratio = parameters.pixelRatio;
    // auto paintParamsUniformBuffer = parameters.context.createUniformBuffer(&paintParamsUBO, sizeof(paintParamsUBO));

    // const bool pitchWithMap = evaluated.get<CirclePitchAlignment>() == AlignmentType::Map;

    // if (!evaluatedPropsUniformBuffer) {
    //     CircleEvaluatedPropsUBO evaluatedPropsUBO;
    //     evaluatedPropsUBO.color = evaluated.get<CircleColor>().constantOr(Color());
    //     evaluatedPropsUBO.radius = evaluated.get<CircleRadius>().constantOr(0);
    //     evaluatedPropsUBO.blur = evaluated.get<CircleBlur>().constantOr(0);
    //     evaluatedPropsUBO.opacity = evaluated.get<CircleOpacity>().constantOr(0);
    //     evaluatedPropsUBO.stroke_color = evaluated.get<CircleStrokeColor>().constantOr(Color());
    //     evaluatedPropsUBO.stroke_width = evaluated.get<CircleStrokeWidth>().constantOr(0);
    //     evaluatedPropsUBO.stroke_opacity = evaluated.get<CircleStrokeOpacity>().constantOr(0);
    //     evaluatedPropsUBO.scale_with_map = evaluated.get<CirclePitchScale>() == CirclePitchScaleType::Map;
    //     evaluatedPropsUBO.pitch_with_map = pitchWithMap;
    //     evaluatedPropsUniformBuffer = parameters.context.createUniformBuffer(&evaluatedPropsUBO,
    //                                                                          sizeof(evaluatedPropsUBO));
    // }

    // CircleInterpolateUBO interpolateUBO;
    // interpolateUBO.color_t = 0;
    // interpolateUBO.radius_t = 0;
    // interpolateUBO.blur_t = 0;
    // interpolateUBO.opacity_t = 0;
    // interpolateUBO.stroke_color_t = 0;
    // interpolateUBO.stroke_width_t = 0;
    // interpolateUBO.stroke_opacity_t = 0;
    // auto interpolateUniformBuffer = parameters.context.createUniformBuffer(&interpolateUBO, sizeof(interpolateUBO));

    // layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
    //     drawable.mutableUniformBuffers().addOrReplace("CirclePaintParamsUBO", paintParamsUniformBuffer);
    //     drawable.mutableUniformBuffers().addOrReplace("CircleEvaluatedPropsUBO", evaluatedPropsUniformBuffer);
    //     drawable.mutableUniformBuffers().addOrReplace("CircleInterpolateUBO", interpolateUniformBuffer);

    //     if (!drawable.getTileID()) {
    //         return;
    //     }
    //     mat4 matrix = drawable.getMatrix();
    //     const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
    //     const auto tileMat = parameters.matrixForTile(tileID);
    //     matrix::multiply(matrix, drawable.getMatrix(), tileMat);
    //     matrix = tileMat;

    //     CircleDrawableUBO drawableUBO;
    //     drawableUBO.matrix = util::cast<float>(matrix);
    //     drawableUBO.extrude_scale =
    //         pitchWithMap
    //             ? std::array<float, 2>{{tileID.pixelsToTileUnits(1.0f,
    //             static_cast<float>(parameters.state.getZoom())),
    //                                     tileID.pixelsToTileUnits(1.0f,
    //                                     static_cast<float>(parameters.state.getZoom()))}}
    //             : parameters.pixelsToGLUnits;
    //     auto drawableUniformBuffer = parameters.context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
    //     drawable.mutableUniformBuffers().addOrReplace("CircleDrawableUBO", drawableUniformBuffer);
    // });
}

} // namespace mbgl
