#include <mbgl/mtl/render_target.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/offscreen_texture.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/gfx/render_pass.hpp>
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
    offscreenTexture = context.createOffscreenTexture(size, type);
}

RenderTarget::~RenderTarget() {
}

void RenderTarget::upload(gfx::UploadPass& uploadPass) {
    visitLayerGroups(([&](LayerGroupBase& layerGroup) { layerGroup.upload(uploadPass); }));
}

void RenderTarget::render(RenderOrchestrator& orchestrator, const RenderTree& renderTree, PaintParameters& parameters) {
    parameters.renderPass = parameters.encoder->createRenderPass("render target", {*offscreenTexture, Color{0.0f, 0.0f, 0.0f, 1.0f}, {}, {}});
    
    // Run layer tweakers to update any dynamic elements
    visitLayerGroups([&](LayerGroupBase& layerGroup) {
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
    });
    
    parameters.renderPass.reset();
    parameters.encoder->present(*offscreenTexture);
}

} // namespace mtl
} // namespace mbgl
