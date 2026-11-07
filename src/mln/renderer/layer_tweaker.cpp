#include <mln/renderer/layer_tweaker.hpp>

#include <mln/map/transform_state.hpp>
#include <mln/renderer/render_tree.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/shaders/layer_ubo.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/util/containers.hpp>
#include <mln/util/mat4.hpp>

namespace mln {

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
    return getTileMatrix(tileID,
                         parameters.state,
                         parameters.transformParams,
                         translation,
                         anchor,
                         drawable.getOrigin(),
                         nearClipped,
                         inViewportPixelUnits,
                         aligned);
}

mat4 LayerTweaker::getTileMatrix(const UnwrappedTileID& tileID,
                                 const TransformState& transformState,
                                 const TransformParameters& transformParams,
                                 const std::array<float, 2>& translation,
                                 const style::TranslateAnchorType anchor,
                                 const std::optional<mln::Point<double>>& origin,
                                 bool nearClipped,
                                 bool inViewportPixelUnits,
                                 bool aligned) {
    // from RenderTile::prepare
    mat4 tileMatrix;
    transformState.matrixFor(/*out*/ tileMatrix, tileID);
    if (origin.has_value()) {
        matrix::translate(tileMatrix, tileMatrix, origin->x, origin->y, 0);
    }
    multiplyWithProjectionMatrix(
        /*in-out*/ tileMatrix, transformParams, nearClipped, aligned);
    return RenderTile::translateVtxMatrix(
        tileID, tileMatrix, translation, anchor, transformState, inViewportPixelUnits);
}

void LayerTweaker::updateProperties(Immutable<style::LayerProperties> newProps) {
    evaluatedProperties = std::move(newProps);
    propertiesUpdated = true;
}

void LayerTweaker::multiplyWithProjectionMatrix(/*in-out*/ mat4& matrix,
                                                const TransformParameters& transformParams,
                                                bool nearClipped,
                                                bool aligned) {
    // nearClippedMatrix has near plane moved further, to enhance depth buffer precision
    const auto& projMatrixRef = aligned ? transformParams.alignedProjMatrix
                                        : (nearClipped ? transformParams.nearClippedProjMatrix
                                                       : transformParams.projMatrix);
    matrix::multiply(matrix, projMatrixRef, matrix);
}

} // namespace mln
