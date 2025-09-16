#include <mbgl/webgpu/context.hpp>
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
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/webgpu/tile_layer_group.hpp>
#include <mbgl/webgpu/layer_group.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/util/logging.hpp>

#include <mbgl/shaders/webgpu/clipping_mask.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/program_parameters.hpp>

namespace mbgl {
namespace webgpu {

Context::Context(RendererBackend& backend_)
    : gfx::Context(gfx::Context::minimumRequiredVertexBindingCount),
      backend(backend_),
      globalUniformBuffers(std::make_unique<UniformBufferArray>()) {}

Context::~Context() = default;

void Context::beginFrame() {
    // Begin a new frame - WebGPU command recording starts here
    mbgl::Log::Info(mbgl::Event::Render, "WebGPU Context: beginFrame()");
}

void Context::endFrame() {
    // End the frame - submit WebGPU commands
    mbgl::Log::Info(mbgl::Event::Render, "WebGPU Context: endFrame()");
}

void Context::performCleanup() {
    // Clean up unused resources
}

void Context::reduceMemoryUsage() {
    // Free cached resources to reduce memory
}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size size, gfx::TextureChannelDataType type) {
    return std::make_unique<OffscreenTexture>(*this, size, type, true, false);
}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    return std::make_unique<CommandEncoder>(*this);
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
    mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Creating TileLayerGroup: " + name);
    auto tileLayerGroup = std::make_shared<webgpu::TileLayerGroup>(layerIndex, initialCapacity, std::move(name));
    return tileLayerGroup;
}

LayerGroupPtr Context::createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Creating LayerGroup: " + name);
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

bool Context::emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr& ptr, const void* data, std::size_t size, bool persistent) {
    if (!ptr) {
        mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Creating new uniform buffer, size: " + std::to_string(size));
        ptr = createUniformBuffer(data, size, persistent);
        return true;
    }

    // Update existing buffer
    mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Updating uniform buffer, size: " + std::to_string(size));
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
    mbgl::Log::Info(mbgl::Event::Render, "WebGPU Context::bindGlobalUniformBuffers called");
    // In WebGPU, we can't bind buffers globally to the render pass like Metal does.
    // Instead, we need to ensure these buffers are available for drawables to include
    // in their bind groups. The actual binding happens per-drawable.

    // Store a reference to global buffers in the render pass for drawables to access
    auto& webgpuRenderPass = static_cast<webgpu::RenderPass&>(renderPass);
    webgpuRenderPass.setGlobalUniformBuffers(globalUniformBuffers.get());
}

void Context::unbindGlobalUniformBuffers(gfx::RenderPass& renderPass) const noexcept {
    mbgl::Log::Info(mbgl::Event::Render, "WebGPU Context::unbindGlobalUniformBuffers called");
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

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType type, Size size) {
    (void)type;
    (void)size;
    return std::make_unique<RenderbufferResource>();
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    return std::make_unique<DrawScopeResource>(*this);
}

// Buffer creation (aligned with Metal)
BufferResource Context::createBuffer(const void* data,
                                    std::size_t size,
                                    uint32_t usage,
                                    bool isIndexBuffer,
                                    bool persistent) const {
    return BufferResource(const_cast<Context&>(*this), data, size, usage, isIndexBuffer, persistent);
}

// Get reusable tile vertex buffer (aligned with Metal)
const BufferResource& Context::getTileVertexBuffer() {
    if (!tileVertexBuffer) {
        // Create standard tile vertices (-EXTENT to +EXTENT)
        const float extent = util::EXTENT;
        const std::array<float, 8> vertices = {
            -extent, -extent,
             extent, -extent,
            -extent,  extent,
             extent,  extent
        };

        uint32_t usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        tileVertexBuffer = createBuffer(vertices.data(), vertices.size() * sizeof(float), usage, false, true);
    }
    return *tileVertexBuffer;
}

// Get reusable tile index buffer (aligned with Metal)
const BufferResource& Context::getTileIndexBuffer() {
    if (!tileIndexBuffer) {
        // Create standard tile indices
        const std::array<uint16_t, 6> indices = {
            0, 1, 2,
            1, 3, 2
        };

        uint32_t usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
        tileIndexBuffer = createBuffer(indices.data(), indices.size() * sizeof(uint16_t), usage, true, true);
    }
    return *tileIndexBuffer;
}

gfx::AttributeBindingArray Context::getOrCreateVertexBindings(
    gfx::Context&,
    const gfx::AttributeDataType vertexType,
    const size_t vertexAttributeIndex,
    const std::vector<uint8_t>& vertexData,
    const gfx::VertexAttributeArray& defaults,
    const gfx::VertexAttributeArray& overrides,
    gfx::BufferUsageType,
    const std::optional<std::chrono::duration<double>>&,
    std::shared_ptr<VertexBufferResource>&) noexcept {

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
                    createBuffer(sharedRaw->getRawData(), dataSize, usage, false, false)
                );

                // Create attribute binding
                result[index] = {
                    /*.attribute = */ {attr.getDataType(), /*offset=*/0},
                    /*.vertexStride = */ static_cast<uint32_t>(attr.getStride()),
                    /*.vertexBufferResource = */ bufferResource.get(),
                    /*.vertexOffset = */ 0
                };

                mbgl::Log::Info(mbgl::Event::Render, "Created vertex buffer for attribute " +
                               std::to_string(index) + " size=" + std::to_string(dataSize));
            }
        }
    });

    return result;
}

} // namespace webgpu
} // namespace mbgl
