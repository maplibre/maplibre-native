#include <mbgl/renderer/layers/terrain_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_terrain.hpp>
#include <mbgl/shaders/terrain_layer_ubo.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {

using namespace shaders;

void TerrainLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty() || !terrain) {
        return;
    }

    auto& context = parameters.context;

#if defined(DEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    // Get terrain properties; per the style spec, exaggeration is applied as-is
    // (default 1.0 renders true-scale elevation)
    const float exaggeration = terrain->getExaggeration();
    const float elevationOffset = 0.0f;

    // Populate layer-level UBO with terrain properties
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    const TerrainEvaluatedPropsUBO propsUBO = {.unpack = terrain->getDEMUnpackVector(),
                                               .exaggeration = exaggeration,
                                               .elevation_offset = elevationOffset,
                                               .pad1 = 0.0f,
                                               .pad2 = 0.0f};
    layerUniforms.createOrUpdate(idTerrainEvaluatedPropsUBO, &propsUBO, context);

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<TerrainDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
#endif

    // Visit each drawable to populate per-drawable UBOs
    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        // Calculate transformation matrix for this terrain tile
        // This uses the same matrix calculation as other layers
        mat4 matrix = parameters.matrixForTile(tileID);

#if !MLN_UBO_CONSOLIDATION
        auto& drawableUniforms = drawable.mutableUniformBuffers();
#endif

#if MLN_UBO_CONSOLIDATION
        drawableUBOVector[i] = {
#else
        const TerrainDrawableUBO drawableUBO = {
#endif
            .matrix = util::cast<float>(matrix),
            .dem_coords = terrain->getDrawableDemCoords(*drawable.getTileID())
        };

#if !MLN_UBO_CONSOLIDATION
        drawableUniforms.createOrUpdate(idTerrainDrawableUBO, &drawableUBO, context);
#endif

#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const size_t drawableUBOVectorSize = sizeof(TerrainDrawableUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    layerUniforms.set(idTerrainDrawableUBO, drawableUniformBuffer);
#endif
}

} // namespace mbgl
