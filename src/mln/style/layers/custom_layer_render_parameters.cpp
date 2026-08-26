#include <mln/style/layers/custom_layer_render_parameters.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/mat4.hpp>

namespace mln {
namespace style {

CustomLayerRenderParameters::CustomLayerRenderParameters(const mln::PaintParameters& paintParameters) {
    const TransformState& state = paintParameters.state;
    width = state.getSize().width;
    height = state.getSize().height;
    latitude = state.getLatLng().latitude();
    longitude = state.getLatLng().longitude();
    zoom = state.getZoom();
    bearing = util::rad2deg(-state.getBearing());
    pitch = state.getPitch();
    fieldOfView = state.getFieldOfView();
    mat4 projMatrix;
    state.getProjMatrix(projMatrix);
    projectionMatrix = projMatrix;
    nearClippedProjectionMatrix = paintParameters.transformParams.nearClippedProjMatrix;

    globe = state.isGlobeRendering();
    projectionTransition = state.getProjectionTransition();
    const ProjectionData projection = state.getProjectionData(UnwrappedTileID(0, 0, 0));
    globeProjectionMatrix = projection.mainMatrix;
    globeClippingPlane = projection.clippingPlane;
}

} // namespace style
} // namespace mln
