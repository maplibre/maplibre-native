#include <mbgl/renderer/layer_tweaker.hpp>

#include <mbgl/map/transform_state.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/containers.hpp>

#include <cmath>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/util/monotonic_timer.hpp>
#include <chrono>
#endif // MLN_RENDER_BACKEND_METAL

namespace mbgl {

LayerTweaker::LayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
    : id(std::move(id_)),
      evaluatedProperties(std::move(properties)) {}

bool LayerTweaker::checkTweakDrawable(const gfx::Drawable& drawable) const {
    // Apply to a drawable if it references us, or if doesn't reference anything.
    const auto& tweaker = drawable.getLayerTweaker();
    return !tweaker || tweaker.get() == this;
}

mat4 LayerTweaker::getTileMatrix(const UnwrappedTileID& tileID,
                                 const PaintParameters& parameters,
                                 const std::array<float, 2>& translation,
                                 style::TranslateAnchorType anchor,
                                 bool nearClipped,
                                 bool inViewportPixelUnits,
                                 const gfx::Drawable& drawable,
                                 bool aligned,
                                 bool renderToTerrain,
                                 bool* renderingToTerrain) {
    if (renderToTerrain && parameters.terrain) {
        if (renderingToTerrain) {
            *renderingToTerrain = true;
        }
        // Tile-local orthographic matrix; the placement into each (possibly
        // zoom-mismatched) terrain render target happens in the vertex shader
        // (apply_drape_transform) from the drawable and target tile ids.
        mat4 terrainRttPosMatrix;
        matrix::ortho(terrainRttPosMatrix, 0, util::EXTENT, util::EXTENT, 0, 0, 1);
#if !MLN_RENDER_BACKEND_OPENGL
        // matrix::ortho emits an OpenGL clip volume (z ∈ [-1, 1]); Vulkan, Metal and
        // WebGPU expect z ∈ [0, 1]. Draped 2D geometry has in_position.z = 0, so its
        // clip z is exactly this matrix's constant term (element 14), which the GL
        // ortho sets to -1 — behind the near plane on those backends, so every draped
        // fragment is clipped away and the drape target stays empty (the map draped
        // over terrain renders blank). Remap clip z from [-1, 1] to [0, 1] the usual
        // way, z' = (z + w) / 2, by halving row 2 and folding in row 3 (w). The drape
        // drawables render with depth disabled, so the exact z is irrelevant beyond
        // being inside the clip volume. (element 10 is overwritten just below, but it
        // multiplies in_position.z = 0 and never affects the result.)
        terrainRttPosMatrix[2] = 0.5 * (terrainRttPosMatrix[2] + terrainRttPosMatrix[3]);
        terrainRttPosMatrix[6] = 0.5 * (terrainRttPosMatrix[6] + terrainRttPosMatrix[7]);
        terrainRttPosMatrix[10] = 0.5 * (terrainRttPosMatrix[10] + terrainRttPosMatrix[11]);
        terrainRttPosMatrix[14] = 0.5 * (terrainRttPosMatrix[14] + terrainRttPosMatrix[15]);
#endif
        // Draped 2D geometry multiplies vec4(x, y, 0, 1), so the matrix's third
        // column never contributes to the transform; carry the drawable's tile
        // (z, x, y — x including the wrap) there for apply_drape_transform,
        // which pairs it with the target tile in GlobalPaintParamsUBO::drape_tile.
        const double z = tileID.canonical.z;
        terrainRttPosMatrix[8] = z;
        terrainRttPosMatrix[9] = tileID.canonical.x + tileID.wrap * std::exp2(z);
        terrainRttPosMatrix[10] = tileID.canonical.y;
        return terrainRttPosMatrix;
    }
    if (renderingToTerrain) {
        *renderingToTerrain = false;
    }
    // from RenderTile::prepare
    mat4 tileMatrix;
    parameters.state.matrixFor(/*out*/ tileMatrix, tileID);
    if (const auto& origin{drawable.getOrigin()}; origin.has_value()) {
        matrix::translate(tileMatrix, tileMatrix, origin->x, origin->y, 0);
    }
    multiplyWithProjectionMatrix(/*in-out*/ tileMatrix, parameters, drawable, nearClipped, aligned);
    return RenderTile::translateVtxMatrix(
        tileID, tileMatrix, translation, anchor, parameters.state, inViewportPixelUnits);
}

uint64_t LayerTweaker::propertiesEpoch = 0;

void LayerTweaker::updateProperties(Immutable<style::LayerProperties> newProps) {
    evaluatedProperties = std::move(newProps);
    propertiesUpdated = true;
    ++propertiesEpoch;
}

void LayerTweaker::multiplyWithProjectionMatrix(/*in-out*/ mat4& matrix,
                                                const PaintParameters& parameters,
                                                [[maybe_unused]] const gfx::Drawable& drawable,
                                                bool nearClipped,
                                                bool aligned) {
    // nearClippedMatrix has near plane moved further, to enhance depth buffer precision
    const auto& projMatrixRef = aligned ? parameters.transformParams.alignedProjMatrix
                                        : (nearClipped ? parameters.transformParams.nearClippedProjMatrix
                                                       : parameters.transformParams.projMatrix);
#if !MLN_RENDER_BACKEND_OPENGL
    // If this drawable is participating in depth testing, offset the
    // projection matrix NDC depth range for the drawable's layer and sublayer.
    if (!drawable.getIs3D() && drawable.getEnableDepth()) {
        // copy and adjust the projection matrix
        mat4 projMatrix = projMatrixRef;
        projMatrix[14] -= ((1 + parameters.currentLayer) * PaintParameters::numSublayers -
                           drawable.getSubLayerIndex()) *
                          PaintParameters::depthEpsilon;
        // multiply with the copy
        matrix::multiply(matrix, projMatrix, matrix);
        // early return
        return;
    }
#endif
    matrix::multiply(matrix, projMatrixRef, matrix);
}

} // namespace mbgl
