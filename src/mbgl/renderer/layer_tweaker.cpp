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

#if MLN_RENDER_BACKEND_METAL
void LayerTweaker::setPropertiesAsUniforms(std::vector<std::string> props) {
    if (props != propertiesAsUniforms) {
        propertiesAsUniforms = std::move(props);
        propertiesChanged = true;
    }
}
bool LayerTweaker::hasPropertyAsUniform(const std::string_view attrName) const {
    return propertiesAsUniforms.end() !=
           std::find_if(propertiesAsUniforms.begin(), propertiesAsUniforms.end(), [&](const auto& name) {
               return name.size() + 2 == attrName.size() && 0 == std::strcmp(name.data(), attrName.data() + 2);
           });
}
#endif // MLN_RENDER_BACKEND_METAL

void LayerTweaker::enableOverdrawInspector(bool value) {
    if (overdrawInspector != value) {
        overdrawInspector = value;
        propertiesChanged = true;
    }
}

} // namespace mbgl
