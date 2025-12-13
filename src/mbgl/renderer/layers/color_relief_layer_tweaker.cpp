#include <mbgl/renderer/layers/color_relief_layer_tweaker.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/color_relief_drawable_data.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/style/layers/color_relief_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace shaders;

void ColorReliefLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    const auto& props = static_cast<const style::ColorReliefLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;

    if (layerGroup.empty()) {
        return;
    }

    // Update evaluated properties UBO
    ColorReliefEvaluatedPropsUBO evaluatedPropsUBO;
    evaluatedPropsUBO.opacity = evaluated.get<style::ColorReliefOpacity>();
    evaluatedPropsUBO.pad_eval0 = 0.0f;
    evaluatedPropsUBO.pad_eval1 = 0.0f;
    evaluatedPropsUBO.pad_eval2 = 0.0f;

    auto& context = parameters.context;
    context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &evaluatedPropsUBO);

    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idColorReliefEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<ColorReliefDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<ColorReliefTilePropsUBO> tilePropsUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

#if MLN_UBO_CONSOLIDATION
        drawableUBOVector[i].matrix = util::cast<float>(parameters.matrixForTile(tileID));
        
        // Get tile props from drawable data (set during creation)
        if (const auto& data = drawable.getData()) {
            const auto& colorReliefData = static_cast<const gfx::ColorReliefDrawableData&>(*data);
            tilePropsUBOVector[i] = colorReliefData.tileProps;
        }
        
        drawable.setUBOIndex(i++);
#else
        ColorReliefDrawableUBO drawableUBO;
        drawableUBO.matrix = util::cast<float>(parameters.matrixForTile(tileID));

        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idColorReliefDrawableUBO, &drawableUBO, context);
        // Tile props UBO is set during drawable creation, doesn't change per frame
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const size_t drawableUBOVectorSize = sizeof(ColorReliefDrawableUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    const size_t tilePropsUBOVectorSize = sizeof(ColorReliefTilePropsUBO) * tilePropsUBOVector.size();
    if (!tilePropsUniformBuffer || tilePropsUniformBuffer->getSize() < tilePropsUBOVectorSize) {
        tilePropsUniformBuffer = context.createUniformBuffer(
            tilePropsUBOVector.data(), tilePropsUBOVectorSize, false, true);
    } else {
        tilePropsUniformBuffer->update(tilePropsUBOVector.data(), tilePropsUBOVectorSize);
    }

    layerUniforms.set(idColorReliefDrawableUBO, drawableUniformBuffer);
    layerUniforms.set(idColorReliefTilePropsUBO, tilePropsUniformBuffer);
#endif
}

} // namespace mbgl
