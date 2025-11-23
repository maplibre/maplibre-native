#include <mbgl/renderer/layers/color_relief_layer_tweaker.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/style/layers/color_relief_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace shaders;

void ColorReliefLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto& props = static_cast<const style::ColorReliefLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;

    if (layerGroup.empty()) {
        return;
    }

    // Update evaluated properties UBO
    evaluatedPropsUBO.opacity = evaluated.get<style::ColorReliefOpacity>();

    auto& layerUniforms = context.mutableUniformBuffers();
    layerUniforms.createOrUpdate(idColorReliefEvaluatedPropsUBO, &evaluatedPropsUBO, context);

    // Update each drawable
    for (auto& drawable : layerGroup.getDrawables()) {
        if (!drawable || !drawable->getTileID()) {
            continue;
        }

        const UnwrappedTileID tileID = drawable->getTileID()->toUnwrapped();

        // Update drawable UBO (transform matrix)
        drawableUBO.matrix = parameters.matrixForTile(tileID);

        auto& drawableUniforms = drawable->mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idColorReliefDrawableUBO, &drawableUBO, context);

        // Tile props UBO is set during drawable creation, doesn't change per frame
    }
}

} // namespace mbgl
