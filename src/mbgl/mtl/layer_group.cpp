#include <mbgl/mtl/layer_group.hpp>

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/drawable.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace mtl {

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_, bool renderToTerrain_)
    : mbgl::LayerGroup(layerIndex_, initialCapacity, std::move(name_), renderToTerrain_) {}

void LayerGroup::upload(gfx::UploadPass& uploadPass) {
    if (!enabled) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    visitDrawables([&](gfx::Drawable& drawable) {
        if (drawable.getEnabled()) {
            auto& drawableMTL = static_cast<mtl::Drawable&>(drawable);
            drawableMTL.upload(uploadPass);
        }
    });
}

void LayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    auto& context = static_cast<Context&>(parameters.context);
    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);
    const auto& renderable = renderPass.getDescriptor().renderable;

    // mtl::Drawable::draw sets no depth-stencil state for 3D drawables - the layer
    // group owns it, so all its 3D drawables share one stencilModeFor3D value (see
    // TileLayerGroup::render, which has always done this). This plain layer group
    // never did its part, so 3D drawables here - the terrain surface mesh and its
    // depth pass - drew with the encoder's DEFAULT state: depth test Always, write
    // off. Draw order then decided visibility, and the terrain skirts (later in the
    // index buffer than the surface) painted over it; the render tests showed the
    // drape tiles' black border texels smeared down every curtain. GL and Vulkan
    // pick depthModeFor3D in the drawable itself, so only Metal was affected.
    bool features3d = false;
    bool stencil3d = false;
    visitDrawables([&](const gfx::Drawable& drawable) {
        if (drawable.getEnabled() && drawable.getIs3D() && drawable.hasRenderPass(parameters.pass)) {
            features3d = true;
            if (drawable.getEnableStencil()) {
                stencil3d = true;
            }
        }
    });

    // Stencil-based states can't be cached across frames: stencilModeFor3D hands out
    // a new reference value per call. The depth-only ones persist in the members.
    std::optional<MTLDepthStencilStatePtr> stateStencil;
    std::optional<MTLDepthStencilStatePtr> stateDepthStencil;
    gfx::StencilMode stencilMode3d;
    if (stencil3d) {
        stencilMode3d = parameters.stencilModeFor3D();
        renderPass.getMetalEncoder()->setStencilReferenceValue(stencilMode3d.ref);
    }
    const auto getDepthStencilState = [&](bool depth, bool stencil) -> const MTLDepthStencilStatePtr& {
        auto& state = depth ? (stencil ? stateDepthStencil : stateDepth) : (stencil ? stateStencil : stateNone);
        if (!state) {
            state = context.makeDepthStencilState(depth ? parameters.depthModeFor3D() : gfx::DepthMode::disabled(),
                                                  stencil ? stencilMode3d : gfx::StencilMode::disabled(),
                                                  renderable);
        }
        return *state;
    };

    bool bindUBOs = false;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        if (!bindUBOs) {
            uniformBuffers.bindMtl(renderPass);
            bindUBOs = true;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        // 2D drawables set their own depth-stencil state inside draw()
        if (features3d && drawable.getIs3D()) {
            renderPass.setDepthStencilState(
                getDepthStencilState(drawable.getEnableDepth(), drawable.getEnableStencil()));
        }

        drawable.draw(parameters);
    });
}

} // namespace mtl
} // namespace mbgl
