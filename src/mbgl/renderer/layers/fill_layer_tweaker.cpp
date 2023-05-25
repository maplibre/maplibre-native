#include <mbgl/renderer/layers/fill_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) FillDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> world;
    std::array<float, 2> padding;
};
static_assert(sizeof(FillDrawableUBO) % 16 == 0);

void FillLayerTweaker::execute(LayerGroup& layerGroup, const PaintParameters& parameters) {
    // const auto& evaluated = static_cast<const FillLayerProperties&>(*evaluatedProperties).evaluated;

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getTileID()) {
            return;
        }
        /*mat4 matrix = drawable.getMatrix();
        if (drawable.getTileID()) {
            const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
            const auto tileMat = parameters.matrixForTile(tileID);
            matrix::multiply(matrix, drawable.getMatrix(), tileMat);
            matrix = tileMat;
        }

        const auto renderableSize = parameters.backend.getDefaultRenderable().getSize();

        gfx::DrawableUBO drawableUBO;
        drawableUBO.matrix = util::cast<float>(matrix);
        drawableUBO.world = {(float)renderableSize.width, (float)renderableSize.height};
        auto uniformBuffer = context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
        drawable.mutableUniformBuffers().addOrReplace("DrawableUBO", uniformBuffer);*/

        mat4 matrix = drawable.getMatrix();
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto tileMat = parameters.matrixForTile(tileID);
        matrix::multiply(matrix, drawable.getMatrix(), tileMat);
        matrix = tileMat;

        const auto renderableSize = parameters.backend.getDefaultRenderable().getSize();

        FillDrawableUBO drawableUBO;
        drawableUBO.matrix = util::cast<float>(matrix);
        drawableUBO.world = {(float)renderableSize.width, (float)renderableSize.height};
        auto drawableUniformBuffer = parameters.context.createUniformBuffer(&drawableUBO, sizeof(drawableUBO));
        drawable.mutableUniformBuffers().addOrReplace("FillDrawableUBO", drawableUniformBuffer);
    });
}

} // namespace mbgl
