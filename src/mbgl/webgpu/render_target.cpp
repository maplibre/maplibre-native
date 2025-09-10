#include <mbgl/webgpu/render_target.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/util/size.hpp>

namespace mbgl {

RenderTarget::RenderTarget(gfx::Context& context_, const Size size, const gfx::TextureChannelDataType type)
    : context(context_) {
    // Create offscreen texture for WebGPU render target
    offscreenTexture = context.createOffscreenTexture(size, type);
}

RenderTarget::~RenderTarget() = default;

const gfx::Texture2DPtr& RenderTarget::getTexture() {
    return offscreenTexture ? offscreenTexture->getTexture() : emptyTexture;
}

bool RenderTarget::addLayerGroup(LayerGroupBasePtr layerGroup, bool replace) {
    if (!layerGroup) {
        return false;
    }
    
    const auto layerIndex = layerGroup->getLayerIndex();
    auto it = layerGroupsByLayerIndex.find(layerIndex);
    
    if (it != layerGroupsByLayerIndex.end() && !replace) {
        return false;
    }
    
    layerGroupsByLayerIndex[layerIndex] = std::move(layerGroup);
    return true;
}

bool RenderTarget::removeLayerGroup(const int32_t layerIndex) {
    return layerGroupsByLayerIndex.erase(layerIndex) > 0;
}

size_t RenderTarget::numLayerGroups() const noexcept {
    return layerGroupsByLayerIndex.size();
}

const LayerGroupBasePtr& RenderTarget::getLayerGroup(const int32_t layerIndex) const {
    static const LayerGroupBasePtr nullGroup;
    auto it = layerGroupsByLayerIndex.find(layerIndex);
    return it != layerGroupsByLayerIndex.end() ? it->second : nullGroup;
}

void RenderTarget::upload(gfx::UploadPass& uploadPass) {
    visitLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.upload(uploadPass);
    });
}

void RenderTarget::render(RenderOrchestrator& orchestrator, const RenderTree& renderTree, PaintParameters& parameters) {
    // Render each layer group
    visitLayerGroups([&](LayerGroupBase& layerGroup) {
        layerGroup.render(orchestrator, parameters);
    });
}

namespace {
gfx::Texture2DPtr emptyTexture;
}

} // namespace mbgl