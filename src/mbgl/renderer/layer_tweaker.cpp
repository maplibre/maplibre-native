#include <mbgl/renderer/layer_tweaker.hpp>

#include <mbgl/map/transform_state.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/containers.hpp>

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
                                 bool aligned) {
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
