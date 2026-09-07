#include <mln/mtl/layer_group.hpp>

#include <mln/gfx/drawable_tweaker.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/upload_pass.hpp>
#include <mln/mtl/context.hpp>
#include <mln/mtl/drawable.hpp>
#include <mln/mtl/render_pass.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/shaders/mtl/shader_program.hpp>
#include <mln/util/convert.hpp>

namespace mln {
namespace mtl {

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mln::LayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

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

    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);
    auto& context = static_cast<Context&>(parameters.context);
    const auto& renderable = renderPass.getDescriptor().renderable;

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

        if (drawable.getIs3D()) {
            const MTLDepthStencilStatePtr* state = nullptr;
            if (!drawable.getEnableDepth()) {
                if (!stateNone) {
                    stateNone = context.makeDepthStencilState(
                        gfx::DepthMode::disabled(), gfx::StencilMode::disabled(), renderable);
                }
                state = &*stateNone;
            } else if (drawable.getDepthType() == gfx::DepthMaskType::ReadOnly) {
                if (!stateDepthReadOnly) {
                    stateDepthReadOnly = context.makeDepthStencilState(
                        parameters.depthModeFor3D(gfx::DepthMaskType::ReadOnly),
                        gfx::StencilMode::disabled(),
                        renderable);
                }
                state = &*stateDepthReadOnly;
            } else {
                if (!stateDepthReadWrite) {
                    stateDepthReadWrite = context.makeDepthStencilState(
                        parameters.depthModeFor3D(gfx::DepthMaskType::ReadWrite),
                        gfx::StencilMode::disabled(),
                        renderable);
                }
                state = &*stateDepthReadWrite;
            }
            renderPass.setDepthStencilState(*state);
        }

        drawable.draw(parameters);
    });
}

} // namespace mtl
} // namespace mln
