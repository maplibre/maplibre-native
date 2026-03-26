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
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace mtl {

LayerGroup::LayerGroup(int32_t layerIndex_, std::size_t initialCapacity, std::string name_)
    : mbgl::LayerGroup(layerIndex_, initialCapacity, std::move(name_)) {}

void LayerGroup::upload(gfx::UploadPass& uploadPass) {
    if (getName() == "terrain") {
        mbgl::Log::Info(mbgl::Event::Render,
                        "LayerGroup::upload for terrain, enabled=" + std::to_string(enabled) +
                            ", drawableCount=" + std::to_string(getDrawableCount()));
    }

    if (!enabled) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = uploadPass.createDebugGroup(getName() + "-upload");
#endif

    int uploadedCount = 0;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (drawable.getEnabled()) {
            auto& drawableMTL = static_cast<mtl::Drawable&>(drawable);
            drawableMTL.upload(uploadPass);
            uploadedCount++;
        }
    });

    if (getName() == "terrain") {
        mbgl::Log::Info(mbgl::Event::Render,
                        "LayerGroup::upload for terrain uploaded " + std::to_string(uploadedCount) + " drawables");
    }
}

void LayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    // Terrain debug logging BEFORE early return
    if (getName() == "terrain") {
        mbgl::Log::Info(mbgl::Event::Render,
                        "LayerGroup::render for terrain ENTRY, enabled=" + std::to_string(enabled) +
                            ", drawableCount=" + std::to_string(getDrawableCount()) +
                            ", hasRenderPass=" + std::to_string(parameters.renderPass != nullptr) +
                            ", pass=" + std::to_string(static_cast<int>(parameters.pass)));
    }

    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // Terrain debug logging
    if (getName() == "terrain") {
        mbgl::Log::Info(mbgl::Event::Render,
                        "LayerGroup::render for terrain, drawableCount=" + std::to_string(getDrawableCount()) +
                            ", pass=" + std::to_string(static_cast<int>(parameters.pass)) +
                            " (Pass3D=4, should match!)");
    }

    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);

    bool bindUBOs = false;
    int drawnCount = 0;
    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            if (getName() == "terrain") {
                mbgl::Log::Info(mbgl::Event::Render,
                                "Terrain drawable skipped: enabled=" + std::to_string(drawable.getEnabled()) +
                                    ", hasPass=" + std::to_string(drawable.hasRenderPass(parameters.pass)));
            }
            return;
        }
        drawnCount++;

        if (!bindUBOs) {
            uniformBuffers.bindMtl(renderPass);
            bindUBOs = true;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        drawable.draw(parameters);
    });

    if (getName() == "terrain") {
        mbgl::Log::Info(mbgl::Event::Render,
                        "LayerGroup::render for terrain drew " + std::to_string(drawnCount) + " drawables");
    }
}

} // namespace mtl
} // namespace mbgl
