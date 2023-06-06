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

void TileLayerGroupGL::upload(gfx::UploadPass& uploadPass) {
    if (!enabled) {
        return;
    }

    observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled()) {
            return;
        }

        auto& drawableGL = static_cast<gl::DrawableGL&>(drawable);

#if !defined(NDEBUG)
        std::string label;
        if (const auto& tileID = drawable.getTileID()) {
            label = drawable.getName() + "/" + util::toString(*tileID);
        }
        const auto labelPtr = (label.empty() ? drawable.getName() : label).c_str();
        const auto debugGroup = uploadPass.createDebugGroup(labelPtr);
#endif

        drawableGL.upload(uploadPass);
    });
}

void TileLayerGroupGL::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled) {
        return;
    }

    observeDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        // If this drawable can render either opaque or translucent...
        if (drawable.hasAllRenderPasses(RenderPass::Opaque | RenderPass::Translucent)) {
            // Render it only in the translucent pass if we're below the cutoff, and only in the opaque pass otherwise
            //  (parameters.currentLayer >= parameters.opaquePassCutoff) ? RenderPass::Opaque : RenderPass::Translucent;
            if ((parameters.currentLayer < parameters.opaquePassCutoff && parameters.pass != RenderPass::Translucent) ||
                (parameters.currentLayer >= parameters.opaquePassCutoff && parameters.pass != RenderPass::Opaque)) {
                return;
            }
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
