#include <mbgl/renderer/layer_tweaker.hpp>

#include <mbgl/map/transform_state.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/util/mat4.hpp>

namespace mbgl {

LayerTweaker::LayerTweaker(Immutable<style::LayerProperties> properties)
    : evaluatedProperties(std::move(properties)) {}

mat4 LayerTweaker::getTileMatrix(const UnwrappedTileID& tileID,
                                 const RenderTree& renderTree,
                                 const TransformState& state,
                                 const std::array<float, 2>& translation,
                                 style::TranslateAnchorType anchor,
                                 bool nearClipped,
                                 bool inViewportPixelUnits,
                                 bool aligned) {
    // from RenderTile::prepare
    mat4 tileMatrix;
    state.matrixFor(/*out*/ tileMatrix, tileID);

    const auto& transformParams = renderTree.getParameters().transformParams;
    // nearClippedMatrix has near plane moved further, to enhance depth buffer precision
    const auto& projMatrix = aligned
                                 ? transformParams.alignedProjMatrix
                                 : (nearClipped ? transformParams.nearClippedProjMatrix : transformParams.projMatrix);
    matrix::multiply(tileMatrix, projMatrix, tileMatrix);

    return RenderTile::translateVtxMatrix(tileID, tileMatrix, translation, anchor, state, inViewportPixelUnits);
}

void LayerTweaker::updateProperties(Immutable<style::LayerProperties> newProps) {
    evaluatedProperties = std::move(newProps);
    propertiesUpdated = true;
}

} // namespace mbgl
