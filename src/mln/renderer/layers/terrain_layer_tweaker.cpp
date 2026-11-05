#include <mln/renderer/layers/terrain_layer_tweaker.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/math/angles.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/shaders/terrain_layer_ubo.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/convert.hpp>
#include <mln/util/mat4.hpp>
#include <mln/util/logging.hpp>

#include <algorithm>
#include <cmath>

namespace mln {

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

    // Skirt depth (u_ele_delta): the shader drops the mesh's skirt vertices by
    // this many metres into a curtain that hides the cracks between neighbouring
    // tiles at different zoom levels. ~1/5 of the tile's world width at this zoom,
    // matching maplibre-gl-js Terrain.getSkirtLength().
    const auto zoom = std::max(parameters.state.getZoom(), 0.0);
    const float elevationOffset = static_cast<float>(util::M2PI * util::EARTH_RADIUS_M / std::pow(2.0, zoom) / 5.0);

    // Distance haze (Map::setTerrainHaze), as maplibre-gl-js's terrain fog with fog-ground-blend 0.5, its
    // near plane at n, the camera-to-center distance, and its far plane where the haze is to be full,
    // `range` times n. (gl-js puts its fog near plane at the sea-level point under the center; the center
    // itself keeps the reach the same multiple of n over high ground.) gl-js's fog depth at view depth d,
    // far / (far - n) * (1 - n / d), is clear up to 0.5 and full at 1; the shader takes the haze as
    // clamp(a - b / d, 0, 1) squared, with a = (far + n) / (far - n) and b = 2 far n / (far - n). As in
    // gl-js it fades in between pitches of 60 and 70 degrees. Off, a and b are 0 and the color is clear.
    const auto& state = parameters.state;
    const double hazeNear = state.getCameraToCenterDistance();
    const double hazeFar = hazeNear * terrain->getHazeRange();
    const double hazeFade = std::clamp((state.getPitch() - util::deg2rad(60.0)) / util::deg2rad(10.0), 0.0, 1.0);
    const bool hazy = hazeFar > hazeNear && hazeFade > 0.0;
    const Color hazeColor = hazy ? terrain->getHazeColor() * static_cast<float>(hazeFade) : Color{};
    const double hazeSpan = hazeFar - hazeNear;
    const std::array<float, 2> hazeRamp{hazy ? static_cast<float>((hazeFar + hazeNear) / hazeSpan) : 0.0f,
                                        hazy ? static_cast<float>(2.0 * hazeFar * hazeNear / hazeSpan) : 0.0f};

    // Populate layer-level UBO with terrain properties
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    const TerrainEvaluatedPropsUBO propsUBO = {
        .unpack = terrain->getDEMUnpackVector(),
        .exaggeration = exaggeration,
        .elevation_offset = elevationOffset,
        .pad1 = 0.0f,
        .pad2 = 0.0f,
        .haze_color = {hazeColor.r, hazeColor.g, hazeColor.b, hazeColor.a},
        .haze_ramp = hazeRamp,
        .pad3 = 0.0f,
        .pad4 = 0.0f};
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
        // This uses the same matrix calculation as other layers. The metres->world
        // pixels elevation scale (pixelsPerMeter) is baked into the projection matrix
        // in TransformState::getProjMatrix, so it applies here and to the elevated
        // symbol / circle layers consistently, matching maplibre-gl-js.
        mat4 matrix = parameters.matrixForTile(tileID);

#if !MLN_RENDER_BACKEND_OPENGL
        // matrixForTile builds a GL-convention projection (clip z in [-1, 1]); Vulkan,
        // Metal and WebGPU clip to [0, 1]. Remap clip z from [-1, 1] to [0, 1] the usual
        // way, z' = (z + w) / 2, so terrain that projects into the near half of the GL
        // clip volume (z < 0) is not clipped away on those backends. Matches
        // LayerTweaker::getTileMatrix and clipMatrixForTile, which remap the draped / RTT
        // matrices for the same reason. Monotonic, so skirt-vs-surface depth ordering and
        // the packed depth texture used for symbol occlusion are preserved; GL unchanged.
        matrix::remapClipZToUnitRange(matrix);
#endif

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

} // namespace mln
