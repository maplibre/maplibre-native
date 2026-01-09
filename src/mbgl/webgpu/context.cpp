#include <mbgl/webgpu/context.hpp>

#include <webgpu/webgpu.h>

#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <array>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/drawable_builder.hpp>
#include <mbgl/webgpu/draw_scope_resource.hpp>
#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/vertex_attribute.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/webgpu/tile_layer_group.hpp>
#include <mbgl/webgpu/layer_group.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/logging.hpp>

#include <mbgl/shaders/attributes.hpp>
#include <mbgl/shaders/webgpu/clipping_mask.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/gfx/gfx_types.hpp>

namespace mbgl {
namespace webgpu {

namespace {
const auto clipMaskStencilMode = gfx::StencilMode{
    /*.test=*/gfx::StencilMode::Always(),
    /*.ref=*/0,
    /*.mask=*/0xFF,
    /*.fail=*/gfx::StencilOpType::Keep,
    /*.depthFail=*/gfx::StencilOpType::Keep,
    /*.pass=*/gfx::StencilOpType::Replace,
};
const auto clipMaskDepthMode = gfx::DepthMode{.func = gfx::DepthFunctionType::Always,
                                              .mask = gfx::DepthMaskType::ReadOnly};
} // namespace

Context::Context(RendererBackend& backend_)
    : gfx::Context(gfx::Context::minimumRequiredVertexBindingCount),
      backend(backend_),
      globalUniformBuffers(std::make_unique<UniformBufferArray>()) {}

Context::~Context() = default;

void Context::beginFrame() {
    // Begin a new frame - WebGPU command recording starts here
}

void Context::endFrame() {
    // End the frame - submit WebGPU commands
}

void Context::performCleanup() {
    // Clean up unused resources
}

void Context::reduceMemoryUsage() {
    // Free cached resources to reduce memory
}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size size,
                                                                       gfx::TextureChannelDataType type,
                                                                       bool depth,
                                                                       bool stencil) {
    return std::make_unique<OffscreenTexture>(*this, size, type, depth, stencil);
}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size size, gfx::TextureChannelDataType type) {
    return std::make_unique<OffscreenTexture>(*this, size, type, true, false);
}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    auto encoder = std::make_unique<CommandEncoder>(*this);
    currentCommandEncoder = encoder.get();
    return encoder;
}

gfx::VertexAttributeArrayPtr Context::createVertexAttributeArray() const {
    return std::make_shared<VertexAttributeArray>();
}

gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    return std::make_unique<DrawableBuilder>(std::move(name));
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data, std::size_t size, bool persistent, bool) {
    (void)persistent;
    return std::make_shared<UniformBuffer>(*this, data, size);
}

gfx::UniqueUniformBufferArray Context::createLayerUniformBufferArray() {
    return std::make_unique<UniformBufferArray>();
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry& registry, const std::string& name) {
    // Align with Metal - just get shader from registry without caching
    const auto shaderGroup = registry.getShaderGroup(name);
    auto shader = shaderGroup ? shaderGroup->getOrCreateShader(*this, {}) : gfx::ShaderProgramBasePtr{};
    return std::static_pointer_cast<gfx::ShaderProgramBase>(std::move(shader));
}

TileLayerGroupPtr Context::createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    auto tileLayerGroup = std::make_shared<webgpu::TileLayerGroup>(layerIndex, initialCapacity, std::move(name));
    return tileLayerGroup;
}

LayerGroupPtr Context::createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    auto layerGroup = std::make_shared<webgpu::LayerGroup>(layerIndex, initialCapacity, std::move(name));
    return layerGroup;
}

gfx::Texture2DPtr Context::createTexture2D() {
    return std::make_shared<Texture2D>(*this);
}

RenderTargetPtr Context::createRenderTarget(const Size size, const gfx::TextureChannelDataType type) {
    return std::make_shared<RenderTarget>(*this, size, type);
}

void Context::resetState(gfx::DepthMode, gfx::ColorMode) {
    // Reset WebGPU render state
}

void Context::setDirtyState() {
    // Mark state as needing update
}

void Context::clearStencilBuffer(int32_t value) {
    // Clear stencil buffer to specified value
    (void)value;
}

bool Context::emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr& ptr,
                                           const void* data,
                                           std::size_t size,
                                           bool persistent) {
    if (!ptr) {
        ptr = createUniformBuffer(data, size, persistent);
        return true;
    }

    // Update existing buffer
    auto* buffer = static_cast<UniformBuffer*>(ptr.get());
    buffer->update(data, size);
    return false;
}

const gfx::UniformBufferArray& Context::getGlobalUniformBuffers() const {
    return *globalUniformBuffers;
}

gfx::UniformBufferArray& Context::mutableGlobalUniformBuffers() {
    return *globalUniformBuffers;
}

void Context::bindGlobalUniformBuffers(gfx::RenderPass& renderPass) const noexcept {
    // In WebGPU, we can't bind buffers globally to the render pass like Metal does.
    // Instead, we need to ensure these buffers are available for drawables to include
    // in their bind groups. The actual binding happens per-drawable.

    // Store a reference to global buffers in the render pass for drawables to access
    auto& webgpuRenderPass = static_cast<webgpu::RenderPass&>(renderPass);
    webgpuRenderPass.setGlobalUniformBuffers(globalUniformBuffers.get());
}

void Context::unbindGlobalUniformBuffers(gfx::RenderPass& renderPass) const noexcept {
    // Clear the global buffer reference from the render pass
    auto& webgpuRenderPass = static_cast<webgpu::RenderPass&>(renderPass);
    webgpuRenderPass.setGlobalUniformBuffers(nullptr);
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {
    // Debug visualization of stencil buffer
}

void Context::visualizeDepthBuffer([[maybe_unused]] float depthRangeSize) {
    // Debug visualization of depth buffer
}
#endif

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType type,
                                                                               Size size) {
    (void)type;
    (void)size;
    return std::make_unique<RenderbufferResource>();
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    return std::make_unique<DrawScopeResource>(*this);
}

// Buffer creation (aligned with Metal)
BufferResource Context::createBuffer(
    const void* data, std::size_t size, uint32_t usage, bool isIndexBuffer, bool persistent) const {
    return BufferResource(const_cast<Context&>(*this), data, size, usage, isIndexBuffer, persistent);
}

// Get reusable tile vertex buffer (aligned with Metal)
const BufferResource& Context::getTileVertexBuffer() {
    if (!tileVertexBuffer) {
        const auto vertices = RenderStaticData::tileVertices();
        const uint32_t usage = WGPUBufferUsage_Vertex;
        tileVertexBuffer = createBuffer(vertices.data(), vertices.bytes(), usage, false, true);
    }
    return *tileVertexBuffer;
}

// Get reusable tile index buffer (aligned with Metal)
const BufferResource& Context::getTileIndexBuffer() {
    if (!tileIndexBuffer) {
        const auto indices = RenderStaticData::quadTriangleIndices();
        const uint32_t usage = WGPUBufferUsage_Index;
        tileIndexBuffer = createBuffer(indices.data(), indices.bytes(), usage, true, true);
    }
    return *tileIndexBuffer;
}

bool Context::renderTileClippingMasks(gfx::RenderPass& renderPass,
                                      RenderStaticData& staticData,
                                      const std::vector<shaders::ClipUBO>& tileUBOs) {
    using ShaderClass = shaders::ShaderSource<shaders::BuiltIn::ClippingMaskProgram, gfx::Backend::Type::WebGPU>;

    if (tileUBOs.empty()) {
        return false;
    }

    if (!clipMaskShader) {
        if (auto group = staticData.shaders->getShaderGroup(ShaderClass::name)) {
            clipMaskShader = std::static_pointer_cast<gfx::ShaderProgramBase>(group->getOrCreateShader(*this, {}));
        }
    }
    if (!clipMaskShader) {
        Log::Error(Event::Render, "WebGPU: Failed to acquire clipping mask shader");
        return false;
    }

    auto& shader = static_cast<webgpu::ShaderProgram&>(*clipMaskShader);
    auto& webgpuRenderPass = static_cast<webgpu::RenderPass&>(renderPass);
    const auto encoder = webgpuRenderPass.getEncoder();
    if (!encoder) {
        Log::Error(Event::Render, "WebGPU: render pass encoder unavailable for clipping masks");
        return false;
    }

    const auto& renderable = webgpuRenderPass.getDescriptor().renderable;
    if (clipMaskRenderable != &renderable) {
        clipMaskPipelineHash.reset();
        clipMaskRenderable = &renderable;
    }

    const auto& vertexResource = getTileVertexBuffer();
    const auto& indexResource = getTileIndexBuffer();
    if (!vertexResource.getBuffer() || !indexResource.getBuffer()) {
        Log::Error(Event::Render, "WebGPU: Missing tile geometry buffers for clipping masks");
        return false;
    }

    const gfx::ColorMode colorMode = gfx::ColorMode::disabled();

    WGPUVertexAttribute vertexAttribute{};
    vertexAttribute.format = WGPUVertexFormat_Sint16x2;
    vertexAttribute.offset = 0;
    vertexAttribute.shaderLocation = static_cast<uint32_t>(ShaderClass::attributes[0].index);

    WGPUVertexBufferLayout vertexLayout{};
    vertexLayout.arrayStride = sizeof(gfx::Vertex<PositionOnlyLayoutAttributes>);
    vertexLayout.attributeCount = 1;
    vertexLayout.attributes = &vertexAttribute;
    vertexLayout.stepMode = WGPUVertexStepMode_Vertex;

    if (!clipMaskPipelineHash) {
        clipMaskPipelineHash = util::hash(colorMode.hash(),
                                          vertexLayout.arrayStride,
                                          static_cast<uint32_t>(vertexAttribute.format),
                                          vertexAttribute.shaderLocation);
    }

    const auto pipeline = shader.getRenderPipeline(renderable,
                                                   &vertexLayout,
                                                   1,
                                                   colorMode,
                                                   clipMaskDepthMode,
                                                   clipMaskStencilMode,
                                                   gfx::DrawModeType::Triangles,
                                                   clipMaskPipelineHash);
    if (!pipeline) {
        Log::Error(Event::Render, "WebGPU: Failed to create clipping mask pipeline");
        return false;
    }

    wgpuRenderPassEncoderSetPipeline(encoder, pipeline);

    wgpuRenderPassEncoderSetVertexBuffer(encoder,
                                         /*slot=*/0,
                                         vertexResource.getBuffer(),
                                         /*offset=*/0,
                                         vertexResource.getSizeInBytes());
    wgpuRenderPassEncoderSetIndexBuffer(encoder,
                                        indexResource.getBuffer(),
                                        WGPUIndexFormat_Uint16,
                                        /*offset=*/0,
                                        indexResource.getSizeInBytes());

    const uint64_t uboSize = sizeof(shaders::ClipUBO);

    clipMaskUniformBuffers.clear();
    clipMaskUniformBuffers.reserve(tileUBOs.size());
    for (const auto& ubo : tileUBOs) {
        auto buffer = createBuffer(&ubo, uboSize, WGPUBufferUsage_Uniform, false, false);
        if (!buffer.getBuffer()) {
            Log::Error(Event::Render, "WebGPU: Failed to allocate uniform buffer for clipping mask tile");
            return false;
        }
        clipMaskUniformBuffers.emplace_back(std::move(buffer));
    }

    auto& rendererBackend = static_cast<RendererBackend&>(getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(rendererBackend.getDevice());
    if (!device) {
        Log::Error(Event::Render, "WebGPU: Device unavailable for clipping masks");
        return false;
    }

    const auto& bindGroupOrder = shader.getBindGroupOrder();
    if (bindGroupOrder.empty()) {
        Log::Error(Event::Render, "WebGPU: No bind group layouts available for clipping mask shader");
        return false;
    }
    const uint32_t bindGroupIndex = bindGroupOrder.front();
    const auto layout = shader.getBindGroupLayout(bindGroupIndex);
    if (!layout) {
        Log::Error(Event::Render, "WebGPU: Bind group layout missing for clipping mask shader");
        return false;
    }

    const auto& bindingInfos = shader.getBindingInfosForGroup(bindGroupIndex);
    uint32_t uniformBinding = 0;
    bool bindingFound = false;
    for (const auto& info : bindingInfos) {
        if (info.type == webgpu::ShaderProgram::BindingType::UniformBuffer) {
            uniformBinding = info.binding;
            bindingFound = true;
            break;
        }
    }
    if (!bindingFound) {
        Log::Error(Event::Render, "WebGPU: Uniform binding not found for clipping mask shader");
        return false;
    }

    clipMaskActiveBindGroups.clear();
    clipMaskActiveBindGroups.reserve(tileUBOs.size());

    for (std::size_t i = 0; i < tileUBOs.size(); ++i) {
        WGPUBindGroupEntry entry{};
        entry.binding = uniformBinding;
        const auto& uboBuffer = clipMaskUniformBuffers[i];
        entry.buffer = uboBuffer.getBuffer();
        entry.offset = 0;
        entry.size = uboSize;

        WGPUBindGroupDescriptor descriptor{};
        descriptor.layout = layout;
        descriptor.entryCount = 1;
        descriptor.entries = &entry;

        const auto bindGroup = wgpuDeviceCreateBindGroup(device, &descriptor);
        if (!bindGroup) {
            Log::Error(Event::Render, "WebGPU: Failed to create bind group for clipping mask draw");
            continue;
        }

        clipMaskActiveBindGroups.push_back(bindGroup);

        wgpuRenderPassEncoderSetBindGroup(encoder, bindGroupIndex, bindGroup, 0, nullptr);
        wgpuRenderPassEncoderSetStencilReference(encoder, tileUBOs[i].stencil_ref);

        wgpuRenderPassEncoderDrawIndexed(encoder,
                                         /*indexCount=*/6,
                                         /*instanceCount=*/1,
                                         /*firstIndex=*/0,
                                         /*baseVertex=*/0,
                                         /*firstInstance=*/0);
    }

    for (auto bindGroup : clipMaskActiveBindGroups) {
        if (bindGroup) {
            wgpuBindGroupRelease(bindGroup);
        }
    }
    clipMaskActiveBindGroups.clear();

    auto& renderStats = renderingStats();
    renderStats.numDrawCalls += tileUBOs.size();
    renderStats.totalDrawCalls += tileUBOs.size();

    return true;
}

gfx::AttributeBindingArray Context::getOrCreateVertexBindings(gfx::Context&,
                                                              const gfx::AttributeDataType vertexType,
                                                              const size_t vertexAttributeIndex,
                                                              const std::vector<uint8_t>& vertexData,
                                                              const gfx::VertexAttributeArray& defaults,
                                                              const gfx::VertexAttributeArray& overrides,
                                                              gfx::BufferUsageType,
                                                              const std::optional<std::chrono::duration<double>>&,
                                                              std::shared_ptr<VertexBufferResource>&) noexcept {
    static_cast<void>(vertexType);
    static_cast<void>(vertexAttributeIndex);
    static_cast<void>(vertexData);

    gfx::AttributeBindingArray result;
    result.resize(overrides.allocatedSize());

    // Process each vertex attribute
    overrides.visitAttributes([&](const gfx::VertexAttribute& attr) {
        const auto index = static_cast<std::size_t>(attr.getIndex());
        if (index >= result.size()) return;

        // Get the shared raw data if available
        gfx::VertexVectorBasePtr sharedRaw = attr.getSharedRawData();
        if (!sharedRaw) {
            // Try default attributes by ID
            defaults.visitAttributes([&](const gfx::VertexAttribute& defaultAttr) {
                if (!sharedRaw && defaultAttr.getIndex() == attr.getIndex()) {
                    sharedRaw = defaultAttr.getSharedRawData();
                }
            });
        }

        if (sharedRaw) {
            // Create vertex buffer from the shared data
            const auto dataSize = sharedRaw->getRawSize() * sharedRaw->getRawCount();
            if (dataSize > 0) {
                // Create GPU buffer for this vertex attribute
                uint32_t usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
                auto bufferResource = std::make_shared<VertexBufferResource>(
                    createBuffer(sharedRaw->getRawData(), dataSize, usage, false, false));

                // Create attribute binding
                result[index] = {/*.attribute = */ {attr.getDataType(), /*offset=*/0},
                                 /*.vertexStride = */ static_cast<uint32_t>(attr.getStride()),
                                 /*.vertexBufferResource = */ bufferResource.get(),
                                 /*.vertexOffset = */ 0};
            }
        }
    });

    return result;
}

} // namespace webgpu
} // namespace mbgl
