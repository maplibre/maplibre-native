#include <mln/style/layers/custom_layer_render_parameters.hpp>
#include <mln/renderer/paint_parameters.hpp>
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
}

} // namespace style
} // namespace mln
