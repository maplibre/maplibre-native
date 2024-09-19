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
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/vulkan/vertex_attribute.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>
#include <mbgl/shaders/vulkan/clipping_mask.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/thread_pool.hpp>
#include <mbgl/util/hash.hpp>

#include <glslang/Public/ShaderLang.h>

#include <algorithm>
#include <cstring>

#ifndef MLN_VULKAN_DESCRIPTOR_POOL_SIZE
#define MLN_VULKAN_DESCRIPTOR_POOL_SIZE 10000
#endif

namespace mbgl {
namespace vulkan {

// Maximum number of vertex attributes, per vertex descriptor
// 32 on most devices (~30% Android use 16),
// per https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxVertexInputBindings
// this can be queried at runtime (VkPhysicalDeviceLimits.maxVertexInputBindings)
constexpr uint32_t maximumVertexBindingCount = 16;
static uint32_t glslangRefCount = 0;

class RenderbufferResource : public gfx::RenderbufferResource {
public:
    RenderbufferResource() = default;
};

Context::Context(RendererBackend& backend_)
    : gfx::Context(vulkan::maximumVertexBindingCount),
      backend(backend_) {
    if (glslangRefCount++ == 0) {
        glslang::InitializeProcess();
    }

    initFrameResources();
}

Context::~Context() noexcept {
    backend.getThreadPool().runRenderJobs(true /* closeQueue */);

    destroyResources();

    if (--glslangRefCount == 0) {
        glslang::FinalizeProcess();
    }
}

void Context::initFrameResources() {
    const auto& device = backend.getDevice();
    const auto frameCount = backend.getMaxFrames();

    // command buffers
    const vk::CommandBufferAllocateInfo allocateInfo(
        backend.getCommandPool().get(), vk::CommandBufferLevel::ePrimary, frameCount);

    auto commandBuffers = backend.getDevice()->allocateCommandBuffersUnique(allocateInfo);

    // descriptor pool info
    const std::vector<vk::DescriptorPoolSize> poolSizes = {
        {vk::DescriptorType::eUniformBuffer, MLN_VULKAN_DESCRIPTOR_POOL_SIZE},
        {vk::DescriptorType::eCombinedImageSampler, MLN_VULKAN_DESCRIPTOR_POOL_SIZE},
    };

    const auto descriptorPoolInfo = vk::DescriptorPoolCreateInfo().setPoolSizes(poolSizes).setMaxSets(
        MLN_VULKAN_DESCRIPTOR_POOL_SIZE);

    frameResources.reserve(frameCount);

    for (uint32_t index = 0; index < frameCount; ++index) {
        frameResources.emplace_back(commandBuffers[index],
                                    device->createDescriptorPoolUnique(descriptorPoolInfo),
                                    device->createSemaphoreUnique({}),
                                    device->createSemaphoreUnique({}),
                                    device->createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));

        const auto& frame = frameResources.back();

        backend.setDebugName(frame.commandBuffer.get(), "FrameCommandBuffer_" + std::to_string(index));
        backend.setDebugName(frame.descriptorPool.get(), "DescriptorPool_" + std::to_string(index));
        backend.setDebugName(frame.frameSemaphore.get(), "FrameSemaphore_" + std::to_string(index));
        backend.setDebugName(frame.surfaceSemaphore.get(), "SurfaceSemaphore_" + std::to_string(index));
        backend.setDebugName(frame.flightFrameFence.get(), "FrameFence_" + std::to_string(index));
    }

    // force placeholder texture upload before any descriptor sets
    (void)getDummyTexture();
}

void Context::destroyResources() {
    backend.getDevice()->waitIdle();

    for (auto& frame : frameResources) {
        frame.runDeletionQueue(*this);
    }

    // all resources have unique handles
    frameResources.clear();
}

const vk::UniqueDescriptorPool& Context::getCurrentDescriptorPool() const {
    return frameResources[frameResourceIndex].descriptorPool;
}

void Context::enqueueDeletion(std::function<void(const Context&)>&& function) {
    if (frameResources.empty()) {
        function(*this);
        return;
    }

    frameResources[frameResourceIndex].deletionQueue.push_back(std::move(function));
}

void Context::submitOneTimeCommand(const std::function<void(const vk::UniqueCommandBuffer&)>& function) {
    const vk::CommandBufferAllocateInfo allocateInfo(
        backend.getCommandPool().get(), vk::CommandBufferLevel::ePrimary, 1);

    const auto& device = backend.getDevice();
    const auto& commandBuffers = device->allocateCommandBuffersUnique(allocateInfo);
    auto& commandBuffer = commandBuffers.front();

    backend.setDebugName(commandBuffer.get(), "OneTimeSubmitCommandBuffer");

    commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    function(commandBuffer);
    commandBuffer->end();

    const auto submitInfo = vk::SubmitInfo().setCommandBuffers(commandBuffer.get());

    const auto& fence = device->createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlags()));
    backend.getGraphicsQueue().submit(submitInfo, fence.get());

    constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();
    const vk::Result waitFenceResult = device->waitForFences(1, &fence.get(), VK_TRUE, timeout);
    if (waitFenceResult != vk::Result::eSuccess) {
        mbgl::Log::Error(mbgl::Event::Render, "OneTimeCommand - Wait fence failed");
    }
}

void Context::waitFrame() const {
    const auto& device = backend.getDevice();
    auto& frame = frameResources[frameResourceIndex];
    constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    const vk::Result waitFenceResult = device->waitForFences(1, &frame.flightFrameFence.get(), VK_TRUE, timeout);
    if (waitFenceResult != vk::Result::eSuccess) {
        mbgl::Log::Error(mbgl::Event::Render, "Wait fence failed");
    }
}
void Context::beginFrame() {
    const auto& device = backend.getDevice();
    auto& renderableResource = backend.getDefaultRenderable().getResource<SurfaceRenderableResource>();
    const auto& platformSurface = renderableResource.getPlatformSurface();

    if (platformSurface && surfaceUpdateRequested) {
        renderableResource.recreateSwapchain();

        // we wait for an idle device to recreate the swapchain
        // so it's a good opportunity to delete all queued items
        for (auto& frame : frameResources) {
            device->resetDescriptorPool(frame.descriptorPool.get());
            frame.runDeletionQueue(*this);
        }

        // sync resources with swapchain
        frameResourceIndex = 0;
        surfaceUpdateRequested = false;
    }

    backend.startFrameCapture();

    auto& frame = frameResources[frameResourceIndex];
    constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    waitFrame();

    device->resetDescriptorPool(getCurrentDescriptorPool().get());
    frame.runDeletionQueue(*this);

    if (platformSurface) {
        try {
            const vk::ResultValue acquireImageResult = device->acquireNextImageKHR(
                renderableResource.getSwapchain().get(), timeout, frame.surfaceSemaphore.get(), nullptr);

            if (acquireImageResult.result == vk::Result::eSuccess) {
                renderableResource.setAcquiredImageIndex(acquireImageResult.value);
            } else if (acquireImageResult.result == vk::Result::eSuboptimalKHR) {
                renderableResource.setAcquiredImageIndex(acquireImageResult.value);
                // TODO implement pre-rotation transform for surface orientation
                // requestSurfaceUpdate();
                // beginFrame();
                // return;
            }

        } catch (const vk::OutOfDateKHRError& e) {
            // request an update and restart frame
            requestSurfaceUpdate();
            beginFrame();
            return;
        }
    } else {
        renderableResource.setAcquiredImageIndex(frameResourceIndex);
    }

    frame.commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    frame.commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    backend.getThreadPool().runRenderJobs();
}

void Context::endFrame() {
    frameResourceIndex = (frameResourceIndex + 1) % frameResources.size();
}

void Context::submitFrame() {
    const auto& frame = frameResources[frameResourceIndex];
    frame.commandBuffer->end();

    const auto& device = backend.getDevice();
    const auto& graphicsQueue = backend.getGraphicsQueue();
    auto& renderableResource = backend.getDefaultRenderable().getResource<SurfaceRenderableResource>();
    const auto& platformSurface = renderableResource.getPlatformSurface();

    // submit frame commands
    const vk::PipelineStageFlags waitStageMask[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    auto submitInfo = vk::SubmitInfo().setCommandBuffers(frame.commandBuffer.get());

    if (platformSurface) {
        submitInfo.setSignalSemaphores(frame.frameSemaphore.get())
            .setWaitSemaphores(frame.surfaceSemaphore.get())
            .setWaitDstStageMask(waitStageMask);
    }

    const vk::Result resetFenceResult = device->resetFences(1, &frame.flightFrameFence.get());
    if (resetFenceResult != vk::Result::eSuccess) {
        mbgl::Log::Error(mbgl::Event::Render, "Reset fence failed");
    }

    graphicsQueue.submit(submitInfo, frame.flightFrameFence.get());

    // present rendered frame
    if (platformSurface) {
        const auto acquiredImage = renderableResource.getAcquiredImageIndex();
        const auto presentInfo = vk::PresentInfoKHR()
                                     .setSwapchains(renderableResource.getSwapchain().get())
                                     .setWaitSemaphores(frame.frameSemaphore.get())
                                     .setImageIndices(acquiredImage);

        try {
            const auto& presentQueue = backend.getPresentQueue();
            const vk::Result presentResult = presentQueue.presentKHR(presentInfo);
            if (presentResult == vk::Result::eSuboptimalKHR) {
                // TODO implement pre-rotation transform for surface orientation
                // requestSurfaceUpdate();
            }
        } catch (const vk::OutOfDateKHRError& e) {
            requestSurfaceUpdate();
        }
    }

    backend.endFrameCapture();
}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    const auto& frame = frameResources[frameResourceIndex];
    return std::make_unique<CommandEncoder>(*this, frame.commandBuffer);
}

BufferResource Context::createBuffer(const void* data, std::size_t size, std::uint32_t usage, bool persistent) const {
    return BufferResource(const_cast<Context&>(*this), data, size, usage, persistent);
}

UniqueShaderProgram Context::createProgram(shaders::BuiltIn shaderID,
                                           std::string name,
                                           const std::string_view vertex,
                                           const std::string_view fragment,
                                           const ProgramParameters& programParameters,
                                           const mbgl::unordered_map<std::string, std::string>& additionalDefines) {
    auto program = std::make_unique<ShaderProgram>(
        shaderID, name, vertex, fragment, programParameters, additionalDefines, backend, *observer);
    return program;
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
    throw std::runtime_error("Vulkan TextureResource not implemented");
    return nullptr;
}

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType, Size) {
    return std::make_unique<RenderbufferResource>();
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    throw std::runtime_error("Vulkan DrawScopeResource not implemented");
    return nullptr;
}

gfx::VertexAttributeArrayPtr Context::createVertexAttributeArray() const {
    return std::make_shared<VertexAttributeArray>();
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {}

void Context::visualizeDepthBuffer(float) {}
#endif // !defined(NDEBUG)

void Context::clearStencilBuffer(int32_t) {
    // See `PaintParameters::clearStencil`
    assert(false);
}

void Context::bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept {}

bool Context::renderTileClippingMasks(gfx::RenderPass& renderPass,
                                      RenderStaticData& staticData,
                                      const std::vector<shaders::ClipUBO>& tileUBOs) {
    using ShaderClass = shaders::ShaderSource<shaders::BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Vulkan>;

    if (!clipping.shader) {
        const auto group = staticData.shaders->getShaderGroup("ClippingMaskProgram");
        if (group) {
            clipping.shader = std::static_pointer_cast<gfx::ShaderProgramBase>(group->getOrCreateShader(*this, {}));
        }
    }
    if (!clipping.shader) {
        assert(!"Failed to create shader for clip masking");
        return false;
    }

    // Create a vertex buffer from the fixed tile coordinates
    if (!clipping.vertexBuffer) {
        const auto vertices = RenderStaticData::tileVertices();
        clipping.vertexBuffer.emplace(
            createBuffer(vertices.data(), vertices.bytes(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, false));
    }

    // Create a buffer from the fixed tile indexes
    if (!clipping.indexBuffer) {
        const auto indices = RenderStaticData::quadTriangleIndices();
        clipping.indexBuffer.emplace(
            createBuffer(indices.data(), indices.bytes(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, false));
        clipping.indexCount = 6;
    }

    // build pipeline
    if (clipping.pipelineInfo.inputAttributes.empty()) {
        clipping.pipelineInfo.usePushConstants = true;

        clipping.pipelineInfo.colorBlend = false;
        clipping.pipelineInfo.colorMask = vk::ColorComponentFlags();

        clipping.pipelineInfo.depthTest = false;
        clipping.pipelineInfo.depthWrite = false;

        clipping.pipelineInfo.stencilTest = true;
        clipping.pipelineInfo.stencilFunction = vk::CompareOp::eAlways;
        clipping.pipelineInfo.stencilPass = vk::StencilOp::eReplace;
        clipping.pipelineInfo.dynamicValues.stencilWriteMask = 0b11111111;
        clipping.pipelineInfo.dynamicValues.stencilRef = 0b11111111;

        clipping.pipelineInfo.inputBindings.push_back(
            vk::VertexInputBindingDescription()
                .setBinding(0)
                .setStride(VertexAttribute::getStrideOf(ShaderClass::attributes[0].dataType))
                .setInputRate(vk::VertexInputRate::eVertex));

        clipping.pipelineInfo.inputAttributes.push_back(
            vk::VertexInputAttributeDescription()
                .setBinding(0)
                .setLocation(ShaderClass::attributes[0].index)
                .setFormat(PipelineInfo::vulkanFormat(ShaderClass::attributes[0].dataType)));
    }

    auto& shaderImpl = static_cast<ShaderProgram&>(*clipping.shader);
    auto& renderPassImpl = static_cast<RenderPass&>(renderPass);
    auto& commandBuffer = renderPassImpl.getEncoder().getCommandBuffer();

    clipping.pipelineInfo.setRenderable(renderPassImpl.getDescriptor().renderable);

    const auto& pipeline = shaderImpl.getPipeline(clipping.pipelineInfo);

    commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());
    clipping.pipelineInfo.setDynamicValues(backend, commandBuffer);

    const std::array<vk::Buffer, 1> vertexBuffers = {clipping.vertexBuffer->getVulkanBuffer()};
    const std::array<vk::DeviceSize, 1> offset = {0};

    commandBuffer->bindVertexBuffers(0, vertexBuffers, offset);
    commandBuffer->bindIndexBuffer(clipping.indexBuffer->getVulkanBuffer(), 0, vk::IndexType::eUint16);

    for (const auto& tileInfo : tileUBOs) {
        commandBuffer->setStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, tileInfo.stencil_ref);

        commandBuffer->pushConstants(
            getPushConstantPipelineLayout().get(),
            vk::ShaderStageFlags() | vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(tileInfo.matrix),
            &tileInfo.matrix);
        commandBuffer->drawIndexed(clipping.indexCount, 1, 0, 0, 0);
    }

    stats.numDrawCalls++;
    stats.totalDrawCalls++;
    return true;
}

const std::unique_ptr<BufferResource>& Context::getDummyVertexBuffer() {
    if (!dummyVertexBuffer)
        dummyVertexBuffer = std::make_unique<BufferResource>(
            *this, nullptr, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, false);
    return dummyVertexBuffer;
}

const std::unique_ptr<BufferResource>& Context::getDummyUniformBuffer() {
    if (!dummyUniformBuffer)
        dummyUniformBuffer = std::make_unique<BufferResource>(
            *this, nullptr, 16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, false);
    return dummyUniformBuffer;
}

const std::unique_ptr<Texture2D>& Context::getDummyTexture() {
    if (!dummyTexture2D) {
        const Size size(2, 2);
        const std::vector<Color> data(4ull * size.width * size.height, Color::white());

        dummyTexture2D = std::make_unique<Texture2D>(*this);
        dummyTexture2D->setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::UnsignedByte);
        dummyTexture2D->setSize(size);

        submitOneTimeCommand([&](const vk::UniqueCommandBuffer& commandBuffer) {
            dummyTexture2D->uploadSubRegion(data.data(), size, 0, 0, commandBuffer);
        });
    }

    return dummyTexture2D;
}

const vk::UniqueDescriptorSetLayout& Context::getUniformDescriptorSetLayout() {
    if (!uniformDescriptorSetLayout) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        const auto stageFlags = vk::ShaderStageFlags() | vk::ShaderStageFlagBits::eVertex |
                                vk::ShaderStageFlagBits::eFragment;

        for (size_t i = 0; i < shaders::maxUBOCountPerShader; ++i) {
            bindings.push_back(vk::DescriptorSetLayoutBinding()
                                   .setBinding(i)
                                   .setStageFlags(stageFlags)
                                   .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                   .setDescriptorCount(1));
        }

        const auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindings(bindings);
        uniformDescriptorSetLayout = backend.getDevice()->createDescriptorSetLayoutUnique(
            descriptorSetLayoutCreateInfo);
        backend.setDebugName(uniformDescriptorSetLayout.get(), "UniformDescriptorSetLayout");
    }

    return uniformDescriptorSetLayout;
}

const vk::UniqueDescriptorSetLayout& Context::getImageDescriptorSetLayout() {
    if (!imageDescriptorSetLayout) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;

        for (size_t i = 0; i < shaders::maxTextureCountPerShader; ++i) {
            bindings.push_back(vk::DescriptorSetLayoutBinding()
                                   .setBinding(i)
                                   .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                                   .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                                   .setDescriptorCount(1));
        }

        const auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindings(bindings);
        imageDescriptorSetLayout = backend.getDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
        backend.setDebugName(imageDescriptorSetLayout.get(), "ImageDescriptorSetLayout");
    }

    return imageDescriptorSetLayout;
}

const std::vector<vk::DescriptorSetLayout>& Context::getDescriptorSetLayouts() {
    if (descriptorSetLayouts.empty()) {
        descriptorSetLayouts = {getUniformDescriptorSetLayout().get(), getImageDescriptorSetLayout().get()};
    }

    return descriptorSetLayouts;
}

const vk::UniquePipelineLayout& Context::getGeneralPipelineLayout() {
    if (generalPipelineLayout) return generalPipelineLayout;

    const auto& descriptorLayouts = getDescriptorSetLayouts();

    generalPipelineLayout = backend.getDevice()->createPipelineLayoutUnique(
        vk::PipelineLayoutCreateInfo().setSetLayouts(descriptorLayouts));

    backend.setDebugName(generalPipelineLayout.get(), "PipelineLayout_general");

    return generalPipelineLayout;
}

const vk::UniquePipelineLayout& Context::getPushConstantPipelineLayout() {
    if (pushConstantPipelineLayout) return pushConstantPipelineLayout;

    const auto stages = vk::ShaderStageFlags() | vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    const auto pushConstant = vk::PushConstantRange().setSize(sizeof(matf4)).setStageFlags(stages);

    pushConstantPipelineLayout = backend.getDevice()->createPipelineLayoutUnique(
        vk::PipelineLayoutCreateInfo().setPushConstantRanges(pushConstant));

    backend.setDebugName(pushConstantPipelineLayout.get(), "PipelineLayout_pushConstants");

    return pushConstantPipelineLayout;
}

void Context::FrameResources::runDeletionQueue(const Context& context) {
    for (const auto& function : deletionQueue) function(context);

    deletionQueue.clear();
}

} // namespace vulkan
} // namespace mbgl
