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

void LayerTweaker::updateProperties(Immutable<style::LayerProperties> newProps) {
    evaluatedProperties = std::move(newProps);
    propertiesUpdated = true;
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
