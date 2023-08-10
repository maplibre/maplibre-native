#include <mbgl/mtl/render_target.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/texture2d.hpp>
//#include <mbgl/gl/framebuffer.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/util/logging.hpp>

#include <cassert>

namespace mbgl {
namespace mtl {

using namespace platform;

RenderTarget::RenderTarget(Context& context_, const Size size, const gfx::TextureChannelDataType type)
    : context(context_) {
    texture = context.createTexture2D();
    texture->setSize(size);
    texture->setFormat(gfx::TexturePixelType::RGBA, type);
    texture->setSamplerConfiguration(
        {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
}

RenderTarget::~RenderTarget() {
    /*if (framebuffer) {
        MBGL_CHECK_ERROR(glDeleteFramebuffers(1, &framebuffer->get()));
        framebuffer.reset();
    }*/
}

void RenderTarget::upload(gfx::UploadPass& uploadPass) {
    visitLayerGroups(([&](LayerGroupBase& layerGroup) { layerGroup.upload(uploadPass); }));
}

void RenderTarget::render(RenderOrchestrator& orchestrator,
                          const RenderTree& renderTree,
                          PaintParameters& parameters) {
    texture->create();
    /*if (!framebuffer) {
        framebuffer = std::make_shared<UniqueFramebuffer>(context.createFramebuffer(*texture));
    }

    context.bindFramebuffer = *framebuffer;
    context.activeTextureUnit = 0;
    context.scissorTest = false;
    context.viewport = {0, 0, texture->getSize()};
    context.clear(Color{0.0f, 0.0f, 0.0f, 1.0f}, {}, {});*/

    // Run layer tweakers to update any dynamic elements
    /*visitLayerGroups([&](LayerGroupBase& layerGroup) {
        if (layerGroup.getLayerTweaker()) {
            layerGroup.getLayerTweaker()->execute(layerGroup, renderTree, parameters);
        }
    });

    // draw layer groups, opaque pass
    parameters.pass = mbgl::RenderPass::Opaque;
    parameters.currentLayer = 0;
    parameters.depthRangeSize = 1 - (numLayerGroups() + 2) * parameters.numSublayers * PaintParameters::depthEpsilon;

    visitLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        parameters.currentLayer++;
    });

    // draw layer groups, translucent pass
    parameters.pass = mbgl::RenderPass::Translucent;
    parameters.currentLayer = static_cast<int32_t>(numLayerGroups()) - 1;
    parameters.depthRangeSize = 1 - (numLayerGroups() + 2) * parameters.numSublayers * PaintParameters::depthEpsilon;

    visitLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        if (parameters.currentLayer != 0) {
            parameters.currentLayer--;
        }
    });*/
}

} // namespace mtl
} // namespace mbgl
