#include <mbgl/renderer/render_target.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>

namespace mbgl {

RenderTarget::RenderTarget(gfx::Context& context_, const Size size, const gfx::TextureChannelDataType type)
    : context(context_) {
    offscreenTexture = context.createOffscreenTexture(size, type);
}

RenderTarget::~RenderTarget() {}

const gfx::Texture2DPtr& RenderTarget::getTexture() {
    return offscreenTexture->getTexture();
};

bool RenderTarget::addLayerGroup(LayerGroupBasePtr layerGroup, const bool replace) {
    const auto index = layerGroup->getLayerIndex();
    const auto result = layerGroupsByLayerIndex.insert(std::make_pair(index, LayerGroupBasePtr{}));
    if (result.second) {
        // added
        result.first->second = std::move(layerGroup);
        return true;
    } else {
        // not added
        if (replace) {
            result.first->second = std::move(layerGroup);
            return true;
        } else {
            return false;
        }
    }
}

bool RenderTarget::removeLayerGroup(const int32_t layerIndex) {
    const auto hit = layerGroupsByLayerIndex.find(layerIndex);
    if (hit != layerGroupsByLayerIndex.end()) {
        layerGroupsByLayerIndex.erase(hit);
        return true;
    } else {
        return false;
    }
}

size_t RenderTarget::numLayerGroups() const noexcept {
    return layerGroupsByLayerIndex.size();
}

static const LayerGroupBasePtr no_group;

const LayerGroupBasePtr& RenderTarget::getLayerGroup(const int32_t layerIndex) const {
    const auto hit = layerGroupsByLayerIndex.find(layerIndex);
    return (hit == layerGroupsByLayerIndex.end()) ? no_group : hit->second;
}

void RenderTarget::upload(gfx::UploadPass& uploadPass) {
    visitLayerGroups(([&](LayerGroupBase& layerGroup) { layerGroup.upload(uploadPass); }));
}

void RenderTarget::render(RenderOrchestrator& orchestrator, const RenderTree& renderTree, PaintParameters& parameters) {
    parameters.renderPass = parameters.encoder->createRenderPass("render target",
                                                                 {.renderable = *offscreenTexture,
                                                                  .clearColor = Color{0.0f, 0.0f, 0.0f, 1.0f},
                                                                  .clearDepth = {},
                                                                  .clearStencil = {}});

    const gfx::ScissorRect prevSicssorRect = parameters.scissorRect;
    const auto& size = getTexture()->getSize();
    parameters.scissorRect = {0, 0, size.width, size.height};

    // Run layer tweakers to update any dynamic elements
    parameters.currentLayer = 0;
    visitLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.runTweakers(renderTree, parameters);
        parameters.currentLayer++;
    });

    // draw layer groups, opaque pass
    parameters.pass = RenderPass::Opaque;
    parameters.depthRangeSize = 1 -
                                (numLayerGroups() + 2) * PaintParameters::numSublayers * PaintParameters::depthEpsilon;

    parameters.currentLayer = 0;
    visitLayerGroupsReversed([&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        parameters.currentLayer++;
    });

    // draw layer groups, translucent pass
    parameters.pass = RenderPass::Translucent;
    parameters.depthRangeSize = 1 -
                                (numLayerGroups() + 2) * PaintParameters::numSublayers * PaintParameters::depthEpsilon;

    parameters.currentLayer = static_cast<uint32_t>(numLayerGroups()) - 1;
    visitLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
        if (parameters.currentLayer > 0) {
            parameters.currentLayer--;
        }
    });

    parameters.renderPass.reset();
    parameters.encoder->present(*offscreenTexture);

    parameters.scissorRect = prevSicssorRect;
}

} // namespace mbgl
