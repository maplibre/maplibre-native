#include <mln/renderer/layer_tweaker.hpp>
#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>

#include <mln/map/transform_state.hpp>
#include <mln/renderer/render_tree.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/shaders/layer_ubo.hpp>
#include <mln/style/layer_properties.hpp>
#include <mln/util/containers.hpp>
#include <mln/util/convert.hpp>
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

ProjectionData LayerTweaker::getProjectionData(const UnwrappedTileID& tileID,
                                               const PaintParameters& parameters,
                                               const std::array<float, 2>& translation,
                                               style::TranslateAnchorType anchor,
                                               bool nearClipped,
                                               bool inViewportPixelUnits,
                                               const gfx::Drawable& drawable,
                                               bool aligned) {
    auto data = parameters.state.getProjectionDataForMatrix(
        tileID,
        getTileMatrix(tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits, drawable, aligned));
    if (!inViewportPixelUnits) {
        data.translate = util::cast<double>(
            RenderTile::tileUnitTranslation(tileID, translation, anchor, parameters.state));
    }
    return data;
}

shaders::ProjectionUBO LayerTweaker::toProjectionUBO(const ProjectionData& data) {
    return {.matrix = util::cast<float>(data.mainMatrix),
            .fallback_matrix = util::cast<float>(data.fallbackMatrix),
            .tile_mercator_coords = util::cast<float>(data.tileMercatorCoords),
            .clipping_plane = util::cast<float>(data.clippingPlane),
            .projection_transition = static_cast<float>(data.projectionTransition),
            .pad1 = 0,
            .translate = util::cast<float>(data.translate)};
}

float LayerTweaker::globeExtrudeScale(const UnwrappedTileID& tileID, float zoom, double latitudeScale) {
    return static_cast<float>(tileID.pixelsToTileUnits(1.0f, zoom) /
                              (util::EXTENT * static_cast<double>(1ull << tileID.canonical.z)) * util::M2PI *
                              latitudeScale);
}

#if MLN_UBO_CONSOLIDATION
void LayerTweaker::uploadProjectionUBOs(gfx::UniformBufferArray& layerUniforms,
                                        const std::vector<shaders::ProjectionUBO>& ubos,
                                        gfx::Context& context) {
    const size_t size = sizeof(shaders::ProjectionUBO) * ubos.size();
    if (!projectionUniformBuffer || projectionUniformBuffer->getSize() < size) {
        projectionUniformBuffer = context.createUniformBuffer(ubos.data(), size, false, true);
    } else {
        projectionUniformBuffer->update(ubos.data(), size);
    }
    layerUniforms.set(shaders::idProjectionUBO, projectionUniformBuffer);
}
#endif

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
