#include <mln/gl/layer_group_gl.hpp>

#include <mln/gfx/drawable_tweaker.hpp>
#include <mln/gfx/render_pass.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/upload_pass.hpp>
#include <mln/gl/drawable_gl.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/shaders/gl/shader_program_gl.hpp>
#include <mln/util/convert.hpp>
#include <mln/util/instrumentation.hpp>

#include <unordered_map>

namespace mln {
namespace gl {

using namespace platform;

TileLayerGroupGL::TileLayerGroupGL(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : TileLayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

void TileLayerGroupGL::upload(gfx::UploadPass& uploadPass) {
    MLN_TRACE_FUNC();
    MLN_ZONE_STR(name);

    if (!enabled) {
        return;
    }

    visitDrawables([&](gfx::Drawable& drawable) {
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

    MLN_TRACE_FUNC();

    auto& context = static_cast<gl::Context&>(parameters.context);

    // `stencilModeFor3D` uses a different stencil mask value each time its called, so if the
    // drawables in this layer use 3D stencil mode, we need to set it up here so that all the
    // drawables using the SAME shader end up using the same mode value. Scoping the mode per
    // shader (rather than one mode shared by the whole layer group) matters when a layer group
    // has more than one shader (e.g. a plugin layer type with two shaders sharing one layer): two
    // unrelated shaders' geometry shouldn't dedup against each other's fragments, or one shader's
    // drawables can wrongly reject the other's genuinely-visible fragments purely by draw order.
    // 2D and 3D features in the same layer group is not supported.
    bool features3d = false;
    std::unordered_map<const void*, gfx::StencilMode> stencilModesByShader;

    parameters.stencilClippingAvailable = parameters.renderTargetHasStencilBuffer;

    if (getDrawableCount() && parameters.stencilClippingAvailable) {
        MLN_TRACE_ZONE(clip masks);
#if !defined(NDEBUG)
        const auto label_clip = getName() + (getName().empty() ? "" : "-") + "tile-clip-masks";
        const auto debugGroupClip = parameters.encoder->createDebugGroup(label_clip.c_str());
#endif

        // If we're using stencil clipping, we need to handle 3D features separately
        if (stencilTiles && !stencilTiles->empty()) {
            visitDrawables([&](const gfx::Drawable& drawable) {
                if (drawable.getEnabled() && drawable.getIs3D() && drawable.hasRenderPass(parameters.pass)) {
                    features3d = true;
                }
            });
        }

        // If we're doing 2D stenciling and have any drawables with tile IDs,
        // render each tile into the stencil buffer with a different value. 3D stencil modes are
        // computed lazily per shader below, in the draw loop.
        if (!features3d && stencilTiles && !stencilTiles->empty()) {
            if (!parameters.renderTileClippingMasks(stencilTiles)) {
                parameters.stencilClippingAvailable = false;
            }
        }
    }

    if (!parameters.stencilClippingAvailable) {
        context.setStencilMode(gfx::StencilMode::disabled());
    }

#if !defined(NDEBUG)
    const auto label_render = getName() + (getName().empty() ? "" : "-") + "render";
    const auto debugGroupRender = parameters.encoder->createDebugGroup(label_render.c_str());
#endif

    bool bindUBOs = false;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

#if !defined(NDEBUG)
        std::string label_tile;
        if (const auto& tileID = drawable.getTileID()) {
            label_tile = util::toString(drawable.getID().id()) + "/" + drawable.getName() + "/" +
                         util::toString(*tileID);
        }
        const auto labelPtr = (label_tile.empty() ? drawable.getName() : label_tile).c_str();
        const auto debugGroupTile = parameters.encoder->createDebugGroup(labelPtr);
#endif

        if (!bindUBOs) {
            uniformBuffers.bind();
            bindUBOs = true;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        // For layer groups with 3D features, enable either this drawable's shader's stencil
        // mode (lazily allocated the first time each distinct shader is seen, so different
        // shaders in the same layer group get independent dedup pools) or disable stenciling.
        // 2D drawables will set their own stencil mode within `draw`.
        if (features3d) {
            if (drawable.getEnableStencil()) {
                const void* shaderKey = drawable.getShader().get();
                auto it = stencilModesByShader.find(shaderKey);
                if (it == stencilModesByShader.end()) {
                    it = stencilModesByShader.emplace(shaderKey, parameters.stencilModeFor3D()).first;
                }
                context.setStencilMode(it->second);
            } else {
                context.setStencilMode(gfx::StencilMode::disabled());
            }
        }

        drawable.draw(parameters);
    });

    if (bindUBOs) {
        uniformBuffers.unbind();
    }
}

LayerGroupGL::LayerGroupGL(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : LayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

void LayerGroupGL::upload(gfx::UploadPass& uploadPass) {
    if (!enabled) {
        return;
    }

    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled()) {
            return;
        }

        auto& drawableGL = static_cast<gl::DrawableGL&>(drawable);

#if !defined(NDEBUG)
        const auto debugGroup = uploadPass.createDebugGroup(drawable.getName().c_str());
#endif

        drawableGL.upload(uploadPass);
    });
}

void LayerGroupGL::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled) {
        return;
    }

    auto& context = static_cast<gl::Context&>(parameters.context);
    parameters.stencilClippingAvailable = parameters.renderTargetHasStencilBuffer;
    if (!parameters.stencilClippingAvailable) {
        context.setStencilMode(gfx::StencilMode::disabled());
    }

    bool bindUBOs = false;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

#if !defined(NDEBUG)
        const auto debugGroup = parameters.encoder->createDebugGroup(drawable.getName().c_str());
#endif
        if (!bindUBOs) {
            uniformBuffers.bind();
            bindUBOs = true;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        drawable.draw(parameters);
    });

    if (bindUBOs) {
        uniformBuffers.unbind();
    }
}

} // namespace gl
} // namespace mln
