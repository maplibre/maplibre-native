#include <mbgl/mtl/tile_layer_group.hpp>

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/drawable.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

#include <Metal/Metal.hpp>

namespace mbgl {
namespace mtl {

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
            auto& drawableMTL = static_cast<Drawable&>(drawable);
            drawableMTL.upload(uploadPass);
        }
    });
}

void TileLayerGroup::render(RenderOrchestrator&, PaintParameters& parameters) {
    if (!enabled || !getDrawableCount() || !parameters.renderPass) {
        return;
    }

    auto& context = static_cast<Context&>(parameters.context);
    const auto& renderPass = static_cast<const mtl::RenderPass&>(*parameters.renderPass);
    const auto& encoder = renderPass.getMetalEncoder();
    const auto& renderable = renderPass.getDescriptor().renderable;

    // `stencilModeFor3D` uses a different stencil mask value each time its called, so if the
    // drawables in this layer use 3D stencil mode, we need to set it up here so that all the
    // drawables end up using the same mode value.
    // 2D and 3D features in the same layer group is not supported.
    bool features3d = false;
    bool stencil3d = false;
    gfx::StencilMode stencilMode3d;

    // Collect the tile IDs relevant to stenciling and update the stencil buffer, if necessary.
    std::set<UnwrappedTileID> tileIDs;
    std::size_t numEnabled = 0;
    visitDrawables([&](const gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }
        numEnabled += 1;
        if (drawable.getIs3D()) {
            features3d = true;
            if (drawable.getEnableStencil()) {
                stencil3d = true;
            }
        }
        if (!features3d && drawable.getEnableStencil() && drawable.getTileID()) {
            tileIDs.emplace(drawable.getTileID()->toUnwrapped());
        }
    });

    if (!numEnabled) {
        return;
    }

#if !defined(NDEBUG)
    const auto debugGroupRender = parameters.encoder->createDebugGroup(getName() + "-render");
#endif

    // If we're doing 3D stenciling and have any features
    // to draw, set up the single-value stencil mask.
    // If we're doing 2D stenciling and have any drawables with tile IDs,
    // render each tile into the stencil buffer with a different value.
    MTLDepthStencilStatePtr stateWithStencil, stateWithoutStencil;
    if (features3d) {
        const auto depthMode = parameters.depthModeFor3D();
        if (stencil3d) {
            stencilMode3d = parameters.stencilModeFor3D();
            stateWithStencil = context.makeDepthStencilState(depthMode, stencilMode3d, renderable);
            encoder->setStencilReferenceValue(stencilMode3d.ref);
        }
        stateWithoutStencil = context.makeDepthStencilState(depthMode, gfx::StencilMode::disabled(), renderable);
    } else if (!tileIDs.empty()) {
        parameters.renderTileClippingMasks(tileIDs);
    }

    visitDrawables([&](gfx::Drawable& drawable) {
        if (!drawable.getEnabled() || !drawable.hasRenderPass(parameters.pass)) {
            return;
        }

        for (const auto& tweaker : drawable.getTweakers()) {
            tweaker->execute(drawable, parameters);
        }

        // For layer groups with 3D features, enable either the single-value
        // stencil mode for features with stencil enabled or disable stenciling.
        // 2D drawables will set their own stencil mode within `draw`.
        if (features3d) {
            const auto& state = drawable.getEnableStencil() ? stateWithStencil : stateWithoutStencil;
            if (state) {
                encoder->setDepthStencilState(state.get());
            }
        }

        drawable.draw(parameters);
    });
}

} // namespace mtl
} // namespace mbgl
