#include <mbgl/renderer/layers/background_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/background_layer_properties.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) BackgroundDrawableUBO {
    std::array<float, 4 * 4> matrix;
};
static_assert(sizeof(BackgroundDrawableUBO) % 16 == 0);

struct alignas(16) BackgroundLayerUBO {
    Color color;
    float opacity;
    float padding[3];
};
static_assert(sizeof(BackgroundLayerUBO) % 16 == 0);

struct alignas(16) BackgroundPatternLayerUBO {
    std::array<float, 2> pattern_tl_a;
    std::array<float, 2> pattern_br_a;
    std::array<float, 2> pattern_tl_b;
    std::array<float, 2> pattern_br_b;
    std::array<float, 2> texsize;
    std::array<float, 2> pattern_size_a;
    std::array<float, 2> pattern_size_b;
    std::array<float, 2> pixel_coord_upper;
    std::array<float, 2> pixel_coord_lower;
    float tile_units_to_pixels;
    float scale_a;
    float scale_b;
    float mix;
    float opacity;
};
static_assert(sizeof(BackgroundPatternLayerUBO) % 16 == 0);

void BackgroundLayerTweaker::execute(LayerGroup& layerGroup, const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const BackgroundLayerProperties&>(*evaluatedProperties).evaluated;

    if (!layerUniformBuffer) {
        BackgroundLayerUBO layerUBO;
        layerUBO.color = evaluated.get<BackgroundColor>();
        layerUBO.opacity = evaluated.get<BackgroundOpacity>();
        layerUniformBuffer = parameters.context.createUniformBuffer(&layerUBO, sizeof(layerUBO));
    }

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        drawable.mutableUniformBuffers().addOrReplace("BackgroundLayerUBO", layerUniformBuffer);

        if (!drawable.getTileID()) {
            return;
        }
        mat4 matrix = drawable.getMatrix();
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto tileMat = parameters.matrixForTile(tileID);
        matrix::multiply(matrix, drawable.getMatrix(), tileMat);
        matrix = tileMat;

        BackgroundDrawableUBO drawableUBO;
        drawableUBO.matrix = util::cast<float>(matrix);
        auto drawableUniformBuffer = parameters.context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
        drawable.mutableUniformBuffers().addOrReplace("BackgroundDrawableUBO", drawableUniformBuffer);
    });
}

} // namespace mbgl
