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
#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/vulkan/vertex_attribute.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/thread_pool.hpp>
#include <mbgl/util/hash.hpp>

#include <glslang/Public/ShaderLang.h>

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

    glslang::InitializeProcess();

    initFrameResources();
}

Context::~Context() noexcept {
    destroyResources();

    glslang::FinalizeProcess();
}

void Context::initFrameResources() {

    const auto& device = backend.getDevice();
    const auto frameCount = backend.getMaxFrames();
    
    // command buffers
    const vk::CommandBufferAllocateInfo allocateInfo(
        backend.getCommandPool().get(), 
        vk::CommandBufferLevel::ePrimary, 
        frameCount
    );

    auto& commandBuffers = backend.getDevice()->allocateCommandBuffersUnique(allocateInfo);

    // descriptor pool info
    const std::vector<vk::DescriptorPoolSize> poolSizes = {
        {vk::DescriptorType::eUniformBuffer, 100000},
    };

    const auto descriptorPoolInfo = vk::DescriptorPoolCreateInfo()
        .setPoolSizes(poolSizes)
        .setMaxSets(100000);

    frameResources.reserve(frameCount);

    for (uint32_t index = 0; index < frameCount; ++index) {
        frameResources.emplace_back(
            std::move(commandBuffers[index]),
            std::move(device->createDescriptorPoolUnique(descriptorPoolInfo)),
            std::move(device->createSemaphoreUnique({})),
            std::move(device->createSemaphoreUnique({})),
            std::move(device->createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)))
        );

        const auto& frame = frameResources.back();

        backend.setDebugName(frame.commandBuffer.get(), "FrameCommandBuffer_" + std::to_string(index));
        backend.setDebugName(frame.descriptorPool.get(), "DescriptorPool_" + std::to_string(index));
        backend.setDebugName(frame.frameSemaphore.get(), "FrameSemaphore_" + std::to_string(index));
        backend.setDebugName(frame.surfaceSemaphore.get(), "SurfaceSemaphore_" + std::to_string(index));
        backend.setDebugName(frame.flightFrameFence.get(), "FrameFence_" + std::to_string(index));
    }
}

void Context::destroyResources() {
   
    backend.getDevice()->waitIdle();

    dummyUniformBuffer.reset();
    dummyVertexBuffer.reset();

    for (auto& frame : frameResources)
        frame.runDeletionQueue();

    // all resources have unique handles
    frameResources.clear();
}

void Context::enqueueDeletion(const std::function<void()>& function) {
    if (frameResources.empty()) {
        function();
        return;
    }
    
    frameResources[frameResourceIndex].deletionQueue.push_back(function);
}

void Context::beginFrame() {
    const auto& device = backend.getDevice();
    auto& renderableResource = backend.getDefaultRenderable().getResource<RenderableResource>();
    auto& frame = frameResources[frameResourceIndex];
    constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    const vk::Result waitFenceResult = device->waitForFences(1, &frame.flightFrameFence.get(), VK_TRUE, timeout);
    if (waitFenceResult != vk::Result::eSuccess) {
#ifndef NDEBUG
        mbgl::Log::Error(mbgl::Event::Render, "Wait fence failed");
#endif
    }

    frame.runDeletionQueue();
    device->resetDescriptorPool(getCurrentDescriptorPool().get());

    try {
        const vk::ResultValue acquireImageResult = device->acquireNextImageKHR(
            renderableResource.swapchain.get(), timeout, frame.surfaceSemaphore.get(), nullptr);

        if (acquireImageResult.result == vk::Result::eSuccess)
            renderableResource.acquiredImageIndex = acquireImageResult.value;
        else if (acquireImageResult.result == vk::Result::eSuboptimalKHR)
            backend.recreateSwapchain();

    } catch (vk::OutOfDateKHRError e) {
        backend.recreateSwapchain();
    }

    frame.commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    frame.commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));

    Scheduler::GetBackground()->runRenderJobs();
}

void Context::endFrame() {
    
    const auto& frame = frameResources[frameResourceIndex];
    frame.commandBuffer->end();

    const auto& device = backend.getDevice();
    const auto& graphicsQueue = backend.getGraphicsQueue();
    const auto& presentQueue = backend.getPresentQueue();
    const auto& renderableResource = backend.getDefaultRenderable().getResource<RenderableResource>();

    // submit frame commands
    const vk::PipelineStageFlags waitStageMask[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    const auto& submitInfo = vk::SubmitInfo()
        .setCommandBuffers(frame.commandBuffer.get())
        .setWaitSemaphores(frame.surfaceSemaphore.get())
        .setWaitDstStageMask(waitStageMask)
        .setSignalSemaphores(frame.frameSemaphore.get());

    const vk::Result resetFenceResult = device->resetFences(1, &frame.flightFrameFence.get());
    if (resetFenceResult != vk::Result::eSuccess) {
#ifndef NDEBUG
        mbgl::Log::Error(mbgl::Event::Render, "Reset fence failed");
#endif
    }

    graphicsQueue.submit(submitInfo, frame.flightFrameFence.get());

    // present rendered frame
    const auto& presentInfo = vk::PresentInfoKHR()
        .setSwapchains(renderableResource.swapchain.get())
        .setWaitSemaphores(frame.frameSemaphore.get())
        .setImageIndices(renderableResource.acquiredImageIndex);

    try {
        const vk::Result presentResult = presentQueue.presentKHR(presentInfo);
        if (presentResult == vk::Result::eSuboptimalKHR)
            backend.recreateSwapchain();
    } catch (vk::OutOfDateKHRError e) {
        backend.recreateSwapchain();
    }

    frameResourceIndex = (frameResourceIndex + 1) % frameResources.size();
}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    const auto& frame = frameResources[frameResourceIndex];
    return std::make_unique<CommandEncoder>(*this, frame.commandBuffer);
}

BufferResource Context::createBuffer(const void* data,
                                     std::size_t size,
                                     std::uint32_t usage,
                                     bool persistent) const {
    return BufferResource(const_cast<Context&>(*this), data, size, usage, persistent);
}

UniqueShaderProgram Context::createProgram(std::string name,
    const std::string_view vertex, const std::string_view fragment, const ProgramParameters& programParameters,
    const mbgl::unordered_map<std::string, std::string>& additionalDefines) {
    return std::make_unique<ShaderProgram>(name, vertex, fragment, programParameters, additionalDefines, backend);
}

gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    return std::make_unique<DrawableBuilder>(std::move(name));
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data, std::size_t size, bool persistent) {
    return std::make_shared<UniformBuffer>(createBuffer(data, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, persistent));
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry& shaders, const std::string& name) {
    const auto shaderGroup = shaders.getShaderGroup(name);
    auto shader = shaderGroup ? shaderGroup->getOrCreateShader(*this, {}) : gfx::ShaderProgramBasePtr{};
    return std::static_pointer_cast<gfx::ShaderProgramBase>(std::move(shader));
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
    return std::make_shared<Texture2D>(*this);
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
    return std::make_shared<VertexAttributeArray>();
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

const std::unique_ptr<BufferResource>& Context::getDummyVertexBuffer() {
    if (!dummyVertexBuffer)
        dummyVertexBuffer = std::make_unique<BufferResource>(*this, nullptr, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, false);
    return dummyVertexBuffer;
}

const std::unique_ptr<BufferResource>& Context::getDummyUniformBuffer() {
    if (!dummyUniformBuffer) 
        dummyUniformBuffer = std::make_unique<BufferResource>(*this, nullptr, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, false);
    return dummyUniformBuffer;
}

const vk::UniqueDescriptorSetLayout& Context::getDummyDescriptorSetLayout() {
    if (!dummyDescriptorSetLayout) {
        const auto descriptorSetBinding = vk::DescriptorSetLayoutBinding()
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1);

        const auto& descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
            .setBindings(descriptorSetBinding);

        dummyDescriptorSetLayout = backend.getDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
        backend.setDebugName(dummyDescriptorSetLayout.get(), "DummyDescriptorSetLayout");
    }

    return dummyDescriptorSetLayout;
}

void Context::FrameResources::runDeletionQueue() {
    for (const auto& function : deletionQueue)
        function();

   deletionQueue.clear();
}

} // namespace vulkan
} // namespace mbgl
