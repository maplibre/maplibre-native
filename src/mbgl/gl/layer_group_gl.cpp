#include <mbgl/gl/layer_group_gl.hpp>

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace gl {

TileLayerGroupGL::TileLayerGroupGL(int32_t layerIndex_, std::size_t initialCapacity)
    : TileLayerGroup(layerIndex_, initialCapacity) {}

void TileLayerGroupGL::upload(gfx::Context& context, gfx::UploadPass& uploadPass) {
    observeDrawables([&](gfx::Drawable& drawable) {
        auto& drawableGL = static_cast<gl::DrawableGL&>(drawable);

#if !defined(NDEBUG)
        std::string label;
        if (const auto& tileID = drawable.getTileID()) {
            label = drawable.getName() + "/" + util::toString(*tileID);
        }
        const auto labelPtr = (label.empty() ? drawable.getName() : label).c_str();
        const auto debugGroup = uploadPass.createDebugGroup(labelPtr);
#endif

        drawableGL.upload(context, uploadPass);
    });
}

void TileLayerGroupGL::render(RenderOrchestrator&, PaintParameters& parameters) {
    // TODO: Render tile masks
    // for (auto& layer : renderLayers) {
    //    parameters.renderTileClippingMasks(renderTiles);
    //}

    observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.hasRenderPass(parameters.pass)) {
            return;
        }
        if (drawable.hasRenderPass(RenderPass::Opaque) && parameters.currentLayer >= parameters.opaquePassCutoff) {
            return;
        }

#if !defined(NDEBUG)
        std::string label;
        if (const auto& tileID = drawable.getTileID()) {
            label = drawable.getName() + "/" + util::toString(*tileID);
        }
        const auto labelPtr = (label.empty() ? drawable.getName() : label).c_str();
        const auto debugGroup = parameters.encoder->createDebugGroup(labelPtr);
#endif

        drawable.draw(parameters);
    });
}

} // namespace gl
} // namespace mbgl
