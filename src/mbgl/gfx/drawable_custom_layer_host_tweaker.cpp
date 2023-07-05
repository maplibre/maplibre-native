#include <mbgl/gfx/drawable_custom_layer_host_tweaker.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/context.hpp>

#include <mbgl/gfx/renderable.hpp>

namespace mbgl {
namespace gfx {

void DrawableCustomLayerHostTweaker::execute(gfx::Drawable& /*drawable*/, const PaintParameters& paintParameters) {
    // custom drawing
    auto& context = paintParameters.context;
    const TransformState& state = paintParameters.state;
    context.resetState(paintParameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                       paintParameters.colorModeForRenderPass());

    style::CustomLayerRenderParameters parameters;

    parameters.width = state.getSize().width;
    parameters.height = state.getSize().height;
    parameters.latitude = state.getLatLng().latitude();
    parameters.longitude = state.getLatLng().longitude();
    parameters.zoom = state.getZoom();
    parameters.bearing = util::rad2deg(-state.getBearing());
    parameters.pitch = state.getPitch();
    parameters.fieldOfView = state.getFieldOfView();
    mat4 projMatrix;
    state.getProjMatrix(projMatrix);
    parameters.projectionMatrix = projMatrix;

    host->render(parameters);

    // Reset the view back to our original one, just in case the CustomLayer
    // changed the viewport or Framebuffer.
    paintParameters.backend.getDefaultRenderable().getResource<gfx::RenderableResource>().bind();

    context.setDirtyState();
}

} // namespace gfx
} // namespace mbgl
