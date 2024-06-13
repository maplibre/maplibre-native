#include <mbgl/vulkan/context.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/drawable_builder.hpp>
#include <mbgl/vulkan/offscreen_texture.hpp>
#include <mbgl/vulkan/layer_group.hpp>
#include <mbgl/vulkan/tile_layer_group.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/thread_pool.hpp>
#include <mbgl/util/hash.hpp>

#include <algorithm>
#include <cstring>

namespace mbgl {
namespace vulkan {

// Maximum number of vertex attributes, per vertex descriptor
// 32 on most devices (~30% Android use 16), 
// per https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxVertexInputBindings
// this can be queried at runtime (VkPhysicalDeviceLimits.maxVertexInputBindings)
constexpr uint32_t maximumVertexBindingCount = 16;

Context::Context(RendererBackend& backend_)
    : gfx::Context(vulkan::maximumVertexBindingCount),
      backend(backend_) {

    initCommandBuffers();
    initFrameSyncResources();
}

Context::~Context() noexcept {

}

void Context::initCommandBuffers() {
    const vk::CommandBufferAllocateInfo allocateInfo(
        backend.getCommandPool().get(), 
        vk::CommandBufferLevel::ePrimary, 
        backend.getMaxFrames()
    );

    commandBuffers = backend.getDevice()->allocateCommandBuffersUnique(allocateInfo);
}

void Context::initFrameSyncResources() {

    const uint32_t frameCount = backend.getMaxFrames();
    const auto& device = backend.getDevice();

    surfaceAvailableSemaphore.reserve(frameCount);
    frameFinishedSemaphore.reserve(frameCount);
    flightFrameFences.reserve(frameCount);

    for (uint32_t i = 0; i < frameCount; ++i)
    {
        surfaceAvailableSemaphore.push_back(device->createSemaphoreUnique({}));
        frameFinishedSemaphore.push_back(device->createSemaphoreUnique({}));
        flightFrameFences.push_back(device->createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
    }
}

void Context::beginFrame() {
    const auto& device = backend.getDevice();
    auto& renderableResource = backend.getDefaultRenderable().getResource<RenderableResource>();
    constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    const vk::Result waitFenceResult = device->waitForFences(1, &flightFrameFences[frameResourceIndex].get(), VK_TRUE, timeout);
    if (waitFenceResult != vk::Result::eSuccess) {
#ifndef NDEBUG
        mbgl::Log::Error(mbgl::Event::Render, "Wait fence failed");
#endif
    }

    const vk::ResultValue acquireImageResult = device->acquireNextImageKHR(renderableResource.swapchain.get(), timeout, 
        surfaceAvailableSemaphore[frameResourceIndex].get(), nullptr);
    if (acquireImageResult.result == vk::Result::eSuccess) {
        renderableResource.acquiredImageIndex = acquireImageResult.value;
    } else if (acquireImageResult.result == vk::Result::eSuboptimalKHR ||
               acquireImageResult.result == vk::Result::eErrorOutOfDateKHR ||
               acquireImageResult.result == vk::Result::eErrorSurfaceLostKHR) {
        // TODO recreate swapchain
    } else {
        // TODO errors
    }

    const auto& commandBuffer = commandBuffers[frameResourceIndex];
    commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));

    Scheduler::GetBackground()->runRenderJobs();
}

void Context::endFrame() {
    
    const auto& commandBuffer = commandBuffers[frameResourceIndex];
    commandBuffer->end();

    const auto& device = backend.getDevice();
    const auto& graphicsQueue = backend.getGraphicsQueue();
    const auto& presentQueue = backend.getPresentQueue();
    const auto& renderableResource = backend.getDefaultRenderable().getResource<RenderableResource>();

    // submit frame commands
    const vk::PipelineStageFlags waitStageMask[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    const auto& submitInfo = vk::SubmitInfo()
        .setCommandBufferCount(1)
        .setPCommandBuffers(&commandBuffers[frameResourceIndex].get())
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&surfaceAvailableSemaphore[frameResourceIndex].get())
        .setPWaitDstStageMask(waitStageMask)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&frameFinishedSemaphore[frameResourceIndex].get());

    const vk::Result resetFenceResult = device->resetFences(1, &flightFrameFences[frameResourceIndex].get());
    if (resetFenceResult != vk::Result::eSuccess) {
#ifndef NDEBUG
        mbgl::Log::Error(mbgl::Event::Render, "Reset fence failed");
#endif
    }

    graphicsQueue.submit(submitInfo, flightFrameFences[frameResourceIndex].get());

    // present rendered frame
    const auto& presentInfo = vk::PresentInfoKHR()
        .setSwapchainCount(1)
        .setPSwapchains(&renderableResource.swapchain.get())
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&frameFinishedSemaphore[frameResourceIndex].get())
        .setPImageIndices(&renderableResource.acquiredImageIndex);

    const vk::Result presentResult = presentQueue.presentKHR(presentInfo);

    if (presentResult == vk::Result::eSuboptimalKHR ||
        presentResult == vk::Result::eErrorOutOfDateKHR ||
        presentResult == vk::Result::eErrorSurfaceLostKHR) {
        // TODO recreate swapchain
    }

    frameResourceIndex = (frameResourceIndex + 1) % backend.getMaxFrames();
}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    return std::make_unique<CommandEncoder>(*this, commandBuffers[frameResourceIndex]);
}

void Context::performCleanup() {

}

gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    return nullptr;
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data, std::size_t size, bool persistent) {
    return nullptr;
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry& shaders, const std::string& name) {
    const auto shaderGroup = shaders.getShaderGroup(name);
    //auto shader = shaderGroup ? shaderGroup->getOrCreateShader(*this, {}) : gfx::ShaderProgramBasePtr{};
    return nullptr;//std::static_pointer_cast<gfx::ShaderProgramBase>(std::move(shader));
}

TileLayerGroupPtr Context::createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    return std::make_shared<TileLayerGroup>(layerIndex, initialCapacity, std::move(name));
}

LayerGroupPtr Context::createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    return std::make_shared<LayerGroup>(layerIndex, initialCapacity, name);
}

bool Context::emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr& buffer,
                                           const void* data,
                                           std::size_t size,
                                           bool persistent) {
    if (buffer) {
        buffer->update(data, size);
        return false;
    } else {
        buffer = createUniformBuffer(data, size, persistent);
        return true;
    }
}

gfx::Texture2DPtr Context::createTexture2D() {
    return nullptr;//std::make_shared<Texture2D>(*this);
}

RenderTargetPtr Context::createRenderTarget(const Size size, const gfx::TextureChannelDataType type) {
    return std::make_shared<RenderTarget>(*this, size, type);
}

void Context::resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) {}

void Context::setDirtyState() {}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size size,
                                                                       gfx::TextureChannelDataType type,
                                                                       bool depth,
                                                                       bool stencil) {
    return std::make_unique<OffscreenTexture>(*this, size, type, depth, stencil);
}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size size, gfx::TextureChannelDataType type) {
    return createOffscreenTexture(size, type, false, false);
}

std::unique_ptr<gfx::TextureResource> Context::createTextureResource(Size,
                                                                     gfx::TexturePixelType,
                                                                     gfx::TextureChannelDataType) {
    assert(false);
    return nullptr;
}

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType, Size size) {
    return nullptr;//std::make_unique<RenderbufferResource>();
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    assert(false);
    return nullptr;
}

gfx::VertexAttributeArrayPtr Context::createVertexAttributeArray() const {
    return nullptr; //std::make_shared<VertexAttributeArray>();
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {}

void Context::visualizeDepthBuffer(float depthRangeSize) {}
#endif // !defined(NDEBUG)

void Context::clearStencilBuffer(int32_t) {
    // See `PaintParameters::clearStencil`
    assert(false);
}

void Context::bindGlobalUniformBuffers(gfx::RenderPass& renderPass) const noexcept {

}

} // namespace vulkan
} // namespace mbgl
