#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/context_impl.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/drawable_builder.hpp>
#include <mbgl/webgpu/draw_scope_resource.hpp>
#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/webgpu/renderbuffer.hpp>
#include <mbgl/webgpu/renderbuffer_resource.hpp>
#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/vertex_attribute.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/webgpu/tile_layer_group.hpp>
#include <mbgl/webgpu/layer_group.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

Context::Context(RendererBackend& backend_)
    : gfx::Context(gfx::Context::minimumRequiredVertexBindingCount),
      impl(std::make_unique<Impl>()),
      backend(backend_),
      globalUniformBuffers(std::make_unique<UniformBufferArray>()) {
    
    impl->device = backend.getDevice();
    Log::Info(Event::General, "WebGPU Context created");
}

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
    return std::make_shared<UniformBuffer>(*this, data, size);
}

gfx::UniqueUniformBufferArray Context::createLayerUniformBufferArray() {
    return std::make_unique<UniformBufferArray>();
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry& registry, const std::string& name) {
    auto it = impl->shaderCache.find(name);
    if (it != impl->shaderCache.end()) {
        return it->second;
    }
    
    // Create new shader program
    auto shader = std::make_shared<ShaderProgram>(name);
    impl->shaderCache[name] = shader;
    return shader;
}

TileLayerGroupPtr Context::createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    return std::make_shared<webgpu::TileLayerGroup>(layerIndex, initialCapacity, std::move(name));
}

LayerGroupPtr Context::createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    return std::make_shared<webgpu::LayerGroup>(layerIndex, initialCapacity, std::move(name));
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
}

bool Context::emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr& ptr, const void* data, std::size_t size, bool persistent) {
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

void Context::bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept {
    // Bind global uniform buffers for rendering
}

void Context::unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept {
    // Unbind global uniform buffers
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {
    // Debug visualization of stencil buffer
}

void Context::visualizeDepthBuffer(float depthRangeSize) {
    // Debug visualization of depth buffer
}
#endif

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType type, Size size) {
    return std::make_unique<RenderbufferResource>();
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    return std::make_unique<DrawScopeResource>(*this);
}

} // namespace webgpu
} // namespace mbgl