#include <mbgl/webgpu/tile_layer_group.hpp>

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

#include <optional>

namespace mbgl {
namespace webgpu {

TileLayerGroup::TileLayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::TileLayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

void TileLayerGroup::upload(gfx::UploadPass& uploadPass) {
    if (!enabled || !getDrawableCount()) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    visitDrawables([&](gfx::Drawable& drawable) {
        if (drawable.getEnabled()) {
            auto& drawableWebGPU = static_cast<Drawable&>(drawable);
            drawableWebGPU.upload(uploadPass);
        }
    });
}

void TileLayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);

    // `stencilModeFor3D` increments a shared mask value each call, so we cache the results
    // when a layer contains 3D drawables to ensure consistent state across drawables.
    bool features3d = false;
    bool stencil3d = false;
    std::optional<gfx::DepthMode> depthMode3d;
    std::optional<gfx::StencilMode> stencilMode3d;

    if (stencilTiles && !stencilTiles->empty()) {
        visitDrawables([&](const gfx::Drawable& drawable) {
            if (drawable.getEnabled() && drawable.getIs3D() && drawable.hasRenderPass(parameters.pass)) {
                features3d = true;
                if (drawable.getEnableStencil()) {
                    stencil3d = true;
                }
            }
        });
    }

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    if (features3d) {
        depthMode3d = parameters.depthModeFor3D();
        if (stencil3d) {
            stencilMode3d = parameters.stencilModeFor3D();
        }
    } else if (stencilTiles && !stencilTiles->empty()) {
        // Handle stencil tiles if present (2D stenciling)
        parameters.renderTileClippingMasks(stencilTiles);
    }

    // Rely on drawables to provide their shaders; no layer-level override needed.
    bool bindUBOs = false;
    int visitCount = 0;
    int drawCount = 0;
    const auto initialDrawCalls = parameters.context.renderingStats().numDrawCalls;
    visitDrawables([&](gfx::Drawable& drawable) {
        visitCount++;

        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }
        drawCount++;

        if (!drawable.getShader()) {
            mbgl::Log::Warning(mbgl::Event::Render,
                               "Drawable " + drawable.getName() + " in " + getName() + " missing shader; skipping");
            return;
        }

        if (!bindUBOs) {
            // Bind uniform buffers once for all drawables
            static_cast<webgpu::UniformBufferArray&>(uniformBuffers).bindWebgpu(renderPass);
            bindUBOs = true;
        }

        // Copy shared uniform buffers from layer group to drawable before tweakers
        // These are typically global paint params and evaluated props
        auto& drawableWebGPU = static_cast<webgpu::Drawable&>(drawable);

        // Copy any layer-provided uniform buffers so drawables can build bind groups from them.
        // This includes consolidated per-drawable data (e.g. idFillDrawableUBO) which lives in
        // the layer UBO array and is selected via the UBO index each drawable writes later.
        auto& drawableUniforms = drawableWebGPU.mutableUniformBuffers();
        for (size_t i = 0; i < uniformBuffers.allocatedSize(); ++i) {
            const auto& uniformBuffer = uniformBuffers.get(i);
            if (uniformBuffer) {
                drawableUniforms.set(i, uniformBuffer);
            }
        }

        // Execute tweakers - they create drawable-specific uniform buffers like FillDrawableUBO
        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        if (features3d) {
            const auto depth = (drawable.getEnableDepth() && depthMode3d) ? *depthMode3d : gfx::DepthMode::disabled();
            const auto stencil = (drawable.getEnableStencil() && stencilMode3d) ? *stencilMode3d
                                                                                : gfx::StencilMode::disabled();
            drawableWebGPU.setDepthModeFor3D(depth);
            drawableWebGPU.setStencilModeFor3D(stencil);
        }

        drawable.draw(parameters);
    });

    const auto finalDrawCalls = parameters.context.renderingStats().numDrawCalls;
    if (drawCount > 0 && finalDrawCalls == initialDrawCalls) {
        mbgl::Log::Warning(
            mbgl::Event::Render,
            getName() + " visited " + std::to_string(visitCount) + " drawables but produced no draw calls!");
    }
}

} // namespace webgpu
} // namespace mbgl
