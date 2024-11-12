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

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::LayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

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

    bool bindUBOs = false;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        if (!bindUBOs) {
            bindUniformBuffers(renderPass);
            bindUBOs = true;
        }

        drawable.draw(parameters);
    });

    if (bindUBOs) {
        unbindUniformBuffers(renderPass);
    }
}

void LayerGroup::bindUniformBuffers(RenderPass& renderPass) const noexcept {
    for (size_t id = 0; id < uniformBuffers.allocatedSize(); id++) {
        const auto& uniformBuffer = uniformBuffers.get(id);
        if (!uniformBuffer) continue;
        const auto& buffer = static_cast<UniformBuffer&>(*uniformBuffer.get());
        const auto& resource = buffer.getBufferResource();
        renderPass.bindVertex(resource, 0, id);
        renderPass.bindFragment(resource, 0, id);
    }
}

} // namespace mtl
} // namespace mbgl
