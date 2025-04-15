#include <mbgl/style/layers/custom_layer_render_parameters.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/mat4.hpp>

namespace mbgl {
namespace style {

CustomLayerRenderParameters::CustomLayerRenderParameters(const mbgl::PaintParameters& paintParameters) {
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

    const TransformParameters& params = paintParameters.transformParams;
    nearClippedProjMatrix = params.nearClippedProjMatrix;
}

} // namespace style
} // namespace mbgl
