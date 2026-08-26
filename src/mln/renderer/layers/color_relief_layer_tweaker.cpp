#include <mln/renderer/layers/color_relief_layer_tweaker.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/gfx/color_relief_drawable_data.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/shaders/color_relief_layer_ubo.hpp>
#include <mln/style/layers/color_relief_layer_properties.hpp>
#include <mln/util/convert.hpp>

namespace mln {

using namespace shaders;

void ColorReliefLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    auto& context = parameters.context;
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

    context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &evaluatedPropsUBO);

    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idColorReliefEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<ColorReliefDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<ColorReliefTilePropsUBO> tilePropsUBOVector(layerGroup.getDrawableCount());
    std::vector<ProjectionUBO> projectionUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto projection = parameters.projectionDataForTile(tileID);

#if MLN_UBO_CONSOLIDATION
        drawableUBOVector[i].matrix = util::cast<float>(projection.mainMatrix);
        projectionUBOVector[i] = toProjectionUBO(projection);

        // Get tile props from drawable data (set during creation)
        if (const auto& drawableData = drawable.getData()) {
            if (const auto* data = static_cast<const gfx::ColorReliefDrawableData*>(drawableData.get())) {
                tilePropsUBOVector[i] = data->tileProps;
            }
        }

        drawable.setUBOIndex(i++);
#else
        ColorReliefDrawableUBO drawableUBO;
        drawableUBO.matrix = util::cast<float>(projection.mainMatrix);
        const auto projectionUBO = toProjectionUBO(projection);

        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idColorReliefDrawableUBO, &drawableUBO, context);
        drawableUniforms.createOrUpdate(idProjectionUBO, &projectionUBO, context);
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
    uploadProjectionUBOs(layerUniforms, projectionUBOVector, context);
#endif
}

} // namespace mln
