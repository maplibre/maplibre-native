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
                                 bool inViewportPixelUnits) {
    // from RenderTile::prepare
    mat4 tileMatrix;
    state.matrixFor(tileMatrix, tileID);

    const auto& projMatrix = renderTree.getParameters().transformParams.projMatrix;
    matrix::multiply(tileMatrix, projMatrix, tileMatrix);

    return RenderTile::translateVtxMatrix(tileID, tileMatrix, translation, anchor, state, inViewportPixelUnits);
}

} // namespace mbgl
