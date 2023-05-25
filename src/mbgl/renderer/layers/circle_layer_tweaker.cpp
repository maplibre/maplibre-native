#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layers/circle_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/circle_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) CircleLayerDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> extrude_scale;
};
static_assert(sizeof(CircleLayerDrawableUBO) % 16 == 0);

struct alignas(16) CircleLayerVertexUBO {
    float camera_to_center_distance;
    float device_pixel_ratio;
    int scale_with_map;
    int pitch_with_map;
};
static_assert(sizeof(CircleLayerVertexUBO) % 16 == 0);

struct alignas(16) CircleLayerFragmentUBO {
    Color color;
    float radius;
    float blur;
    float opacity;
    Color stroke_color;
    float stroke_width;
    float stroke_opacity;
};
static_assert(sizeof(CircleLayerVertexUBO) % 16 == 0);

struct alignas(16) CircleLayerInterpolateUBO {
    float color_t;
    float radius_t;
    float blur_t;
    float opacity_t;
    float stroke_color_t;
    float stroke_width_t;
    float stroke_opacity_t;
};
static_assert(sizeof(CircleLayerInterpolateUBO) % 16 == 0);

void CircleLayerTweaker::execute(LayerGroup& layerGroup, const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const CircleLayerProperties&>(*evaluatedProperties).evaluated;
    
    CircleLayerVertexUBO vertexUBO;
    vertexUBO.camera_to_center_distance = parameters.state.getCameraToCenterDistance();
    vertexUBO.device_pixel_ratio = parameters.pixelRatio;
    vertexUBO.scale_with_map = evaluated.get<CirclePitchScale>() == CirclePitchScaleType::Map;
    vertexUBO.pitch_with_map = evaluated.get<CirclePitchAlignment>() == AlignmentType::Map;
    auto vertexUniformBuffer = parameters.context.createUniformBuffer(&vertexUBO, sizeof(vertexUBO));
    
    if (!fragmentUniformBuffer) {
        CircleLayerFragmentUBO fragmentUBO;
        fragmentUBO.color = evaluated.get<CircleColor>().constantOr(Color());
        fragmentUBO.radius = evaluated.get<CircleRadius>().constantOr(0);
        fragmentUBO.blur = evaluated.get<CircleBlur>().constantOr(0);
        fragmentUBO.opacity = evaluated.get<CircleOpacity>().constantOr(0);
        fragmentUBO.stroke_color = evaluated.get<CircleStrokeColor>().constantOr(Color());
        fragmentUBO.stroke_width = evaluated.get<CircleStrokeWidth>().constantOr(0);
        fragmentUBO.stroke_opacity = evaluated.get<CircleStrokeOpacity>().constantOr(0);
        fragmentUniformBuffer = parameters.context.createUniformBuffer(&fragmentUBO, sizeof(fragmentUBO));
    }
    
    CircleLayerInterpolateUBO interpolateUBO;
    interpolateUBO.color_t = 0;
    interpolateUBO.radius_t = 0;
    interpolateUBO.blur_t = 0;
    interpolateUBO.opacity_t = 0;
    interpolateUBO.stroke_color_t = 0;
    interpolateUBO.stroke_width_t = 0;
    interpolateUBO.stroke_opacity_t = 0;
    auto interpolateUniformBuffer = parameters.context.createUniformBuffer(&interpolateUBO, sizeof(interpolateUBO));
    
    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        drawable.mutableUniformBuffers().addOrReplace("CircleLayerVertexUBO", vertexUniformBuffer);
        drawable.mutableUniformBuffers().addOrReplace("CircleLayerFragmentUBO", fragmentUniformBuffer);
        drawable.mutableUniformBuffers().addOrReplace("CircleLayerInterpolateUBO", interpolateUniformBuffer);
        
        if (!drawable.getTileID()) {
            return;
        }
        mat4 matrix = drawable.getMatrix();
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto tileMat = parameters.matrixForTile(tileID);
        matrix::multiply(matrix, drawable.getMatrix(), tileMat);
        matrix = tileMat;

        CircleLayerDrawableUBO drawableUBO;
        drawableUBO.matrix = util::cast<float>(matrix);
        drawableUBO.extrude_scale = vertexUBO.pitch_with_map ? std::array<float, 2>{
            {tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom())),
            tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()))}
        } : parameters.pixelsToGLUnits;
        auto drawableUniformBuffer = parameters.context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
        drawable.mutableUniformBuffers().addOrReplace("CircleLayerDrawableUBO", drawableUniformBuffer);
    });
}

} // namespace mbgl
