#include <mbgl/vulkan/context.hpp>

#include <mbgl/gfx/shader_registry.hpp>
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
#include <mbgl/shaders/program_parameters.hpp>
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

constexpr uint32_t globalDescriptorPoolSize = 3 * 4;
constexpr uint32_t layerDescriptorPoolSize = 3 * 256;
constexpr uint32_t drawableUniformDescriptorPoolSize = 3 * 1024;
constexpr uint32_t drawableImageDescriptorPoolSize = drawableUniformDescriptorPoolSize / 2;

namespace {
uint32_t glslangRefCount = 0;
}

class RenderbufferResource : public gfx::RenderbufferResource {
public:
    RenderbufferResource() = default;
};

Context::Context(RendererBackend& backend_)
    : gfx::Context(vulkan::maximumVertexBindingCount),
      backend(backend_),
      globalUniformBuffers(DescriptorSetType::Global, 0, 0, shaders::globalUBOCount) {
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

#if !defined(NDEBUG)
    Log::Debug(Event::General, "Rendering Stats:\n" + stats.toString("\n"));
#endif

    assert(stats.isZero());
}

void Context::initFrameResources() {
    const auto& device = backend.getDevice();
    const auto& dispatcher = backend.getDispatcher();
    const auto frameCount = backend.getMaxFrames();

    descriptorPoolMap.emplace(DescriptorSetType::Global,
                              DescriptorPoolGrowable(globalDescriptorPoolSize, 0, shaders::globalUBOCount, 0));

    descriptorPoolMap.emplace(
        DescriptorSetType::Layer,
        DescriptorPoolGrowable(
            layerDescriptorPoolSize, shaders::maxSSBOCountPerLayer, shaders::maxUBOCountPerLayer, 0));

    descriptorPoolMap.emplace(
        DescriptorSetType::DrawableUniform,
        DescriptorPoolGrowable(drawableUniformDescriptorPoolSize, 0, shaders::maxUBOCountPerDrawable, 0));

    descriptorPoolMap.emplace(
        DescriptorSetType::DrawableImage,
        DescriptorPoolGrowable(drawableImageDescriptorPoolSize, 0, 0, shaders::maxTextureCountPerShader));

    // command buffers
    const vk::CommandBufferAllocateInfo allocateInfo(
        backend.getCommandPool().get(), vk::CommandBufferLevel::ePrimary, frameCount);

    auto commandBuffers = device->allocateCommandBuffersUnique(allocateInfo, dispatcher);

    frameResources.reserve(frameCount);

    for (uint32_t index = 0; index < frameCount; ++index) {
        frameResources.emplace_back(
            commandBuffers[index],
            device->createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled), nullptr, dispatcher));

        const auto& frame = frameResources.back();

        backend.setDebugName(frame.commandBuffer.get(), "FrameCommandBuffer_" + std::to_string(index));
        backend.setDebugName(frame.flightFrameFence.get(), "FrameFence_" + std::to_string(index));
    }

    // force placeholder texture upload before any descriptor sets
    (void)getDummyTexture();

    buildUniformDescriptorSetLayout(
        globalUniformDescriptorSetLayout, 0, 0, shaders::globalUBOCount, "GlobalUniformDescriptorSetLayout");
    buildUniformDescriptorSetLayout(layerUniformDescriptorSetLayout,
                                    shaders::globalUBOCount,
                                    shaders::maxSSBOCountPerLayer,
                                    shaders::maxUBOCountPerLayer,
                                    "LayerUniformDescriptorSetLayout");
    buildUniformDescriptorSetLayout(drawableUniformDescriptorSetLayout,
                                    shaders::globalUBOCount,
                                    0,
                                    shaders::maxUBOCountPerDrawable,
                                    "DrawableUniformDescriptorSetLayout");
    buildImageDescriptorSetLayout();
}

void Context::destroyResources() {
    backend.getDevice()->waitIdle(backend.getDispatcher());

    for (auto& frame : frameResources) {
        frame.runDeletionQueue(*this);
    }

    // all resources have unique handles
    frameResources.clear();

    globalUniformBuffers.freeDescriptorSets();

    for (size_t i = 0; i < globalUniformBuffers.allocatedSize(); i++) {
        globalUniformBuffers.set(i, nullptr);
    }

    dummyBuffer.reset();
    dummyTexture2D.reset();

    clipping.indexBuffer.reset();
    clipping.vertexBuffer.reset();
}

void Context::enqueueDeletion(std::function<void(Context&)>&& function) {
    if (frameResources.empty()) {
        function(*this);
        return;
    }

    frameResources[frameResourceIndex].deletionQueue.push_back(std::move(function));
}

void Context::submitOneTimeCommand(const std::function<void(const vk::UniqueCommandBuffer&)>& function) const {
    MLN_TRACE_FUNC();

    const vk::CommandBufferAllocateInfo allocateInfo(
        backend.getCommandPool().get(), vk::CommandBufferLevel::ePrimary, 1);

    const auto& device = backend.getDevice();
    const auto& dispatcher = backend.getDispatcher();
    const auto& commandBuffers = device->allocateCommandBuffersUnique(allocateInfo, dispatcher);
    auto& commandBuffer = commandBuffers.front();

    backend.setDebugName(commandBuffer.get(), "OneTimeSubmitCommandBuffer");

    commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit), dispatcher);
    function(commandBuffer);
    commandBuffer->end(dispatcher);

    const auto submitInfo = vk::SubmitInfo().setCommandBuffers(commandBuffer.get());

    const auto& fence = device->createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlags()), nullptr, dispatcher);
    backend.getGraphicsQueue().submit(submitInfo, fence.get(), dispatcher);

    constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();
    const vk::Result waitFenceResult = device->waitForFences(1, &fence.get(), VK_TRUE, timeout, dispatcher);
    if (waitFenceResult != vk::Result::eSuccess) {
        mbgl::Log::Error(mbgl::Event::Render, "OneTimeCommand - Wait fence failed");
    }
}

void Context::requestSurfaceUpdate(bool useDelay) {
    if (surfaceUpdateRequested) {
        if (!useDelay) {
            surfaceUpdateLatency = 0;
        }

        return;
    }

    surfaceUpdateRequested = true;
    surfaceUpdateLatency = useDelay ? backend.getMaxFrames() * 3 : 0;
}

void Context::waitFrame() const {
    MLN_TRACE_FUNC();
    const auto& device = backend.getDevice();
    const auto& dispatcher = backend.getDispatcher();
    auto& frame = frameResources[frameResourceIndex];
    constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    const vk::Result waitFenceResult = device->waitForFences(
        1, &frame.flightFrameFence.get(), VK_TRUE, timeout, dispatcher);
    if (waitFenceResult != vk::Result::eSuccess) {
        mbgl::Log::Error(mbgl::Event::Render, "Wait fence failed");
    }
}

void Context::beginFrame() {
    MLN_TRACE_FUNC();

    frameResourceIndex = (frameResourceIndex + 1) % frameResources.size();

    const auto& device = backend.getDevice();
    const auto& dispatcher = backend.getDispatcher();
    auto& renderableResource = backend.getDefaultRenderable().getResource<SurfaceRenderableResource>();
    const auto& platformSurface = renderableResource.getPlatformSurface();

    // poll for surface transform updates if enabled
    const int32_t surfaceTransformPollingInterval = renderableResource.getSurfaceTransformPollingInterval();
    if (surfaceTransformPollingInterval >= 0 && !surfaceUpdateRequested) {
        if (currentFrameCount > surfaceTransformPollingInterval) {
            if (renderableResource.didSurfaceTransformUpdate()) {
                requestSurfaceUpdate();
            }

            currentFrameCount = 0;
        } else {
            ++currentFrameCount;
        }
    }

    if (platformSurface && surfaceUpdateRequested && --surfaceUpdateLatency <= 0) {
        renderableResource.recreateSwapchain();

        // we wait for an idle device to recreate the swapchain
        // so it's a good opportunity to delete all queued items
        for (auto& frame : frameResources) {
            frame.runDeletionQueue(*this);
        }

        // sync resources with swapchain
        frameResourceIndex = 0;
        surfaceUpdateRequested = false;

        // update renderable size
        if (renderableResource.hasSurfaceTransformSupport()) {
            const auto& extent = renderableResource.getExtent();

            auto& renderable = static_cast<Renderable&>(backend.getDefaultRenderable());
            renderable.setSize({extent.width, extent.height});
        }
    }

    backend.startFrameCapture();

    auto& frame = frameResources[frameResourceIndex];
    constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    waitFrame();

    frame.runDeletionQueue(*this);

    if (platformSurface) {
        MLN_TRACE_ZONE(acquireNextImageKHR);
        try {
            const vk::ResultValue acquireImageResult = device->acquireNextImageKHR(
                renderableResource.getSwapchain().get(),
                timeout,
                renderableResource.getAcquireSemaphore(),
                nullptr,
                dispatcher);

            if (acquireImageResult.result == vk::Result::eSuccess) {
                renderableResource.setAcquiredImageIndex(acquireImageResult.value);
            } else if (acquireImageResult.result == vk::Result::eSuboptimalKHR) {
                renderableResource.setAcquiredImageIndex(acquireImageResult.value);
                requestSurfaceUpdate();
            }

        } catch (const vk::OutOfDateKHRError& e) {
            // request an update and restart frame
            requestSurfaceUpdate(false);
            beginFrame();
            return;
        }
    } else {
        renderableResource.setAcquiredImageIndex(frameResourceIndex);
    }

    frame.commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources, dispatcher);
    frame.commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit), dispatcher);

    backend.getThreadPool().runRenderJobs();
}

void Context::endFrame() {}

void Context::submitFrame() {
    MLN_TRACE_FUNC();
    const auto& dispatcher = backend.getDispatcher();
    const auto& frame = frameResources[frameResourceIndex];
    frame.commandBuffer->end(dispatcher);

    const auto& device = backend.getDevice();
    const auto& graphicsQueue = backend.getGraphicsQueue();
    auto& renderableResource = backend.getDefaultRenderable().getResource<SurfaceRenderableResource>();
    const auto& platformSurface = renderableResource.getPlatformSurface();

    // submit frame commands
    const vk::PipelineStageFlags waitStageMask[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    auto submitInfo = vk::SubmitInfo().setCommandBuffers(frame.commandBuffer.get());

    if (platformSurface) {
        submitInfo.setSignalSemaphores(renderableResource.getPresentSemaphore())
            .setWaitSemaphores(renderableResource.getAcquireSemaphore())
            .setWaitDstStageMask(waitStageMask);
    }

    const vk::Result resetFenceResult = device->resetFences(1, &frame.flightFrameFence.get(), dispatcher);
    if (resetFenceResult != vk::Result::eSuccess) {
        mbgl::Log::Error(mbgl::Event::Render, "Reset fence failed");
    }

    graphicsQueue.submit(submitInfo, frame.flightFrameFence.get(), dispatcher);

    // present rendered frame
    if (platformSurface) {
        const auto acquiredImage = renderableResource.getAcquiredImageIndex();
        const auto presentInfo = vk::PresentInfoKHR()
                                     .setSwapchains(renderableResource.getSwapchain().get())
                                     .setWaitSemaphores(renderableResource.getPresentSemaphore())
                                     .setImageIndices(acquiredImage);

        try {
            const auto& presentQueue = backend.getPresentQueue();
            const vk::Result presentResult = presentQueue.presentKHR(presentInfo, dispatcher);
            if (presentResult == vk::Result::eSuboptimalKHR) {
                requestSurfaceUpdate();
            }
        } catch (const vk::OutOfDateKHRError& e) {
            requestSurfaceUpdate(false);
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

void Context::performCleanup() {
    stats.numDrawCalls = 0;
    ++stats.numFrames;
}

gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    return std::make_unique<DrawableBuilder>(std::move(name));
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data, std::size_t size, bool persistent, bool ssbo) {
    return std::make_shared<UniformBuffer>(createBuffer(
        data, size, ssbo ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, persistent));
}

UniqueUniformBufferArray Context::createLayerUniformBufferArray() {
    return std::make_unique<UniformBufferArray>(
        DescriptorSetType::Layer, shaders::globalUBOCount, shaders::maxSSBOCountPerLayer, shaders::maxUBOCountPerLayer);
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

void Context::bindGlobalUniformBuffers(gfx::RenderPass& renderPass) const noexcept {
    auto& renderPassImpl = static_cast<RenderPass&>(renderPass);
    auto& context = const_cast<Context&>(*this);

    auto& renderableResource = renderPassImpl.getDescriptor().renderable.getResource<SurfaceRenderableResource>();
    if (renderableResource.hasSurfaceTransformSupport()) {
        float surfaceRotation = renderableResource.getRotation();

        const shaders::GlobalPlatformParamsUBO platformUBO = {
            /* .rotation0 = */ {cosf(surfaceRotation), -sinf(surfaceRotation)},
            /* .rotation1 = */ {sinf(surfaceRotation), cosf(surfaceRotation)}};
        context.globalUniformBuffers.createOrUpdate(
            shaders::idGlobalPlatformParamsUBO, &platformUBO, sizeof(platformUBO), context);
    }

    context.globalUniformBuffers.bindDescriptorSets(renderPassImpl.getEncoder());
}

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
                .setStride(static_cast<uint32_t>(VertexAttribute::getStrideOf(ShaderClass::attributes[0].dataType)))
                .setInputRate(vk::VertexInputRate::eVertex));

        clipping.pipelineInfo.inputAttributes.push_back(
            vk::VertexInputAttributeDescription()
                .setBinding(0)
                .setLocation(static_cast<uint32_t>(ShaderClass::attributes[0].index))
                .setFormat(PipelineInfo::vulkanFormat(ShaderClass::attributes[0].dataType)));
    }

    const auto& dispatcher = backend.getDispatcher();
    auto& shaderImpl = static_cast<ShaderProgram&>(*clipping.shader);
    auto& renderPassImpl = static_cast<RenderPass&>(renderPass);
    auto& commandBuffer = renderPassImpl.getEncoder().getCommandBuffer();

    clipping.pipelineInfo.setRenderable(renderPassImpl.getDescriptor().renderable);

    const auto& pipeline = shaderImpl.getPipeline(clipping.pipelineInfo);

    commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get(), dispatcher);
    clipping.pipelineInfo.setScissorRect(
        {0, 0, clipping.pipelineInfo.viewExtent.width, clipping.pipelineInfo.viewExtent.height});
    clipping.pipelineInfo.setDynamicValues(backend, commandBuffer);

    const std::array<vk::Buffer, 1> vertexBuffers = {clipping.vertexBuffer->getVulkanBuffer()};
    const std::array<vk::DeviceSize, 1> offset = {0};

    commandBuffer->bindVertexBuffers(0, vertexBuffers, offset, dispatcher);
    commandBuffer->bindIndexBuffer(clipping.indexBuffer->getVulkanBuffer(), 0, vk::IndexType::eUint16, dispatcher);

    auto& renderableResource = renderPassImpl.getDescriptor().renderable.getResource<SurfaceRenderableResource>();
    const float rad = renderableResource.getRotation();
    const mat4 rotationMat = {
        std::cos(rad), -std::sin(rad), 0, 0, std::sin(rad), std::cos(rad), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    for (const auto& tileInfo : tileUBOs) {
        commandBuffer->setStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, tileInfo.stencil_ref, dispatcher);

        mat4 matrix;
        matrix::multiply(matrix, rotationMat, tileInfo.matrix);
        const auto& matrixf = util::cast<float>(matrix);

        commandBuffer->pushConstants(
            getPushConstantPipelineLayout().get(),
            vk::ShaderStageFlags() | vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(matrixf),
            &matrixf,
            dispatcher);
        commandBuffer->drawIndexed(clipping.indexCount, 1, 0, 0, 0, dispatcher);
    }

    stats.numDrawCalls++;
    stats.totalDrawCalls++;
    return true;
}

const std::unique_ptr<BufferResource>& Context::getDummyBuffer() {
    if (!dummyBuffer) {
        const uint32_t usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        dummyBuffer = std::make_unique<BufferResource>(*this, nullptr, 16, usage, false);
    }

    return dummyBuffer;
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

void Context::buildUniformDescriptorSetLayout(vk::UniqueDescriptorSetLayout& layout,
                                              size_t startId,
                                              size_t storageCount,
                                              size_t uniformCount,
                                              const std::string& name) {
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (size_t i = 0; i < storageCount + uniformCount; ++i) {
        auto stageFlags = vk::ShaderStageFlags();
        if (startId + i != shaders::idDrawableReservedFragmentOnlyUBO) {
            stageFlags |= vk::ShaderStageFlagBits::eVertex;
        }
        if (startId + i != shaders::idDrawableReservedVertexOnlyUBO) {
            stageFlags |= vk::ShaderStageFlagBits::eFragment;
        }

        const auto descriptorType = i < storageCount ? vk::DescriptorType::eStorageBuffer
                                                     : vk::DescriptorType::eUniformBuffer;

        bindings.push_back(vk::DescriptorSetLayoutBinding()
                               .setBinding(static_cast<uint32_t>(i))
                               .setStageFlags(stageFlags)
                               .setDescriptorType(descriptorType)
                               .setDescriptorCount(1));
    }

    const auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindings(bindings);
    layout = backend.getDevice()->createDescriptorSetLayoutUnique(
        descriptorSetLayoutCreateInfo, nullptr, backend.getDispatcher());
    backend.setDebugName(layout.get(), name);
}

void Context::buildImageDescriptorSetLayout() {
    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    for (size_t i = 0; i < shaders::maxTextureCountPerShader; ++i) {
        bindings.push_back(vk::DescriptorSetLayoutBinding()
                               .setBinding(static_cast<uint32_t>(i))
                               .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                               .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                               .setDescriptorCount(1));
    }

    const auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindings(bindings);
    drawableImageDescriptorSetLayout = backend.getDevice()->createDescriptorSetLayoutUnique(
        descriptorSetLayoutCreateInfo, nullptr, backend.getDispatcher());
    backend.setDebugName(drawableImageDescriptorSetLayout.get(), "ImageDescriptorSetLayout");
}

const vk::DescriptorSetLayout& Context::getDescriptorSetLayout(DescriptorSetType type) {
    switch (type) {
        case DescriptorSetType::Global:
            return globalUniformDescriptorSetLayout.get();

        case DescriptorSetType::Layer:
            return layerUniformDescriptorSetLayout.get();

        case DescriptorSetType::DrawableUniform:
            return drawableUniformDescriptorSetLayout.get();

        case DescriptorSetType::DrawableImage:
            return drawableImageDescriptorSetLayout.get();

        default:
            assert(static_cast<uint32_t>(type) < static_cast<uint32_t>(DescriptorSetType::Count));
            return globalUniformDescriptorSetLayout.get();
            break;
    }
}

DescriptorPoolGrowable& Context::getDescriptorPool(DescriptorSetType type) {
    assert(static_cast<uint32_t>(type) < static_cast<uint32_t>(DescriptorSetType::Count));
    return descriptorPoolMap[type];
}

const vk::UniquePipelineLayout& Context::getGeneralPipelineLayout() {
    if (generalPipelineLayout) return generalPipelineLayout;

    const auto stages = vk::ShaderStageFlags() | vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    const auto pushConstant = vk::PushConstantRange().setSize(sizeof(uint32_t)).setStageFlags(stages);

    const std::vector<vk::DescriptorSetLayout> layouts = {
        globalUniformDescriptorSetLayout.get(),
        layerUniformDescriptorSetLayout.get(),
        drawableUniformDescriptorSetLayout.get(),
        drawableImageDescriptorSetLayout.get(),
    };

    generalPipelineLayout = backend.getDevice()->createPipelineLayoutUnique(
        vk::PipelineLayoutCreateInfo().setPushConstantRanges(pushConstant).setSetLayouts(layouts),
        nullptr,
        backend.getDispatcher());

    backend.setDebugName(generalPipelineLayout.get(), "PipelineLayout_general");

    return generalPipelineLayout;
}

const vk::UniquePipelineLayout& Context::getPushConstantPipelineLayout() {
    if (pushConstantPipelineLayout) return pushConstantPipelineLayout;

    const auto stages = vk::ShaderStageFlags() | vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    const auto pushConstant = vk::PushConstantRange().setSize(sizeof(matf4)).setStageFlags(stages);

    pushConstantPipelineLayout = backend.getDevice()->createPipelineLayoutUnique(
        vk::PipelineLayoutCreateInfo().setPushConstantRanges(pushConstant), nullptr, backend.getDispatcher());

    backend.setDebugName(pushConstantPipelineLayout.get(), "PipelineLayout_pushConstants");

    return pushConstantPipelineLayout;
}

void Context::FrameResources::runDeletionQueue(Context& context) {
    MLN_TRACE_FUNC();

    for (const auto& function : deletionQueue) function(context);

    deletionQueue.clear();
}

} // namespace vulkan
} // namespace mbgl
