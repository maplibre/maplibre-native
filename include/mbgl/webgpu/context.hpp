#pragma once

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/buffer_resource.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace mbgl {
class RenderStaticData;
namespace gfx {
// Forward declaration
class AttributeBinding;
using AttributeBindingArray = std::vector<std::optional<AttributeBinding>>;
class RenderPass;
class Renderable;
class ShaderProgramBase;
} // namespace gfx

namespace shaders {
struct ClipUBO;
} // namespace shaders

namespace webgpu {

// Forward declaration
class VertexBufferResource;

class Context : public gfx::Context {
public:
    explicit Context(RendererBackend& backend);
    ~Context() override;

    RendererBackend& getBackend() { return backend; }
    const RendererBackend& getBackend() const { return backend; }

    // Frame management
    void beginFrame() override;
    void endFrame() override;
    void performCleanup() override;
    void reduceMemoryUsage() override;

    // Resource creation
    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType, bool, bool);
    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType) override;
    std::unique_ptr<gfx::CommandEncoder> createCommandEncoder() override;
    gfx::VertexAttributeArrayPtr createVertexAttributeArray() const override;
    gfx::UniqueDrawableBuilder createDrawableBuilder(std::string name) override;
    gfx::UniformBufferPtr createUniformBuffer(const void* data,
                                              std::size_t size,
                                              bool persistent = false,
                                              bool ssbo = false) override;
    gfx::UniqueUniformBufferArray createLayerUniformBufferArray() override;
    gfx::ShaderProgramBasePtr getGenericShader(gfx::ShaderRegistry&, const std::string& name) override;
    TileLayerGroupPtr createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;
    LayerGroupPtr createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;
    gfx::Texture2DPtr createTexture2D() override;
    RenderTargetPtr createRenderTarget(const Size size, const gfx::TextureChannelDataType type) override;

    // State management
    void resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) override;
    void setDirtyState() override;
    void clearStencilBuffer(int32_t) override;

    // Uniform buffer management
    bool emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr&,
                                      const void* data,
                                      std::size_t size,
                                      bool persistent = false) override;
    const gfx::UniformBufferArray& getGlobalUniformBuffers() const override;
    gfx::UniformBufferArray& mutableGlobalUniformBuffers() override;
    void bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;
    void unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;

#if !defined(NDEBUG)
    void visualizeStencilBuffer() override;
    void visualizeDepthBuffer(float depthRangeSize) override;
#endif

    // Buffer creation (aligned with Metal)
    BufferResource createBuffer(
        const void* data, std::size_t size, uint32_t usage, bool isIndexBuffer, bool persistent) const;

    // Get reusable buffers (aligned with Metal)
    const BufferResource& getTileVertexBuffer();
    const BufferResource& getTileIndexBuffer();

    // Tile clipping mask rendering
    bool renderTileClippingMasks(gfx::RenderPass& renderPass,
                                 RenderStaticData& staticData,
                                 const std::vector<shaders::ClipUBO>& tileUBOs);

    // Command encoder tracking
    class CommandEncoder* getCurrentCommandEncoder() const { return currentCommandEncoder; }
    void setCurrentCommandEncoder(class CommandEncoder* encoder) { currentCommandEncoder = encoder; }

    // Vertex binding creation - needed for drawable geometry
    gfx::AttributeBindingArray getOrCreateVertexBindings(gfx::Context&,
                                                         const gfx::AttributeDataType vertexType,
                                                         const size_t vertexAttributeIndex,
                                                         const std::vector<uint8_t>& vertexData,
                                                         const gfx::VertexAttributeArray& defaults,
                                                         const gfx::VertexAttributeArray& overrides,
                                                         gfx::BufferUsageType,
                                                         const std::optional<std::chrono::duration<double>>&,
                                                         std::shared_ptr<webgpu::VertexBufferResource>&) noexcept;

protected:
    std::unique_ptr<gfx::RenderbufferResource> createRenderbufferResource(gfx::RenderbufferPixelType, Size) override;
    std::unique_ptr<gfx::DrawScopeResource> createDrawScopeResource() override;

private:
    RendererBackend& backend;
    std::unique_ptr<gfx::UniformBufferArray> globalUniformBuffers;
    class CommandEncoder* currentCommandEncoder = nullptr;

    // Cached buffers (aligned with Metal)
    std::optional<BufferResource> tileVertexBuffer;
    std::optional<BufferResource> tileIndexBuffer;

    // Cached clipping resources
    gfx::ShaderProgramBasePtr clipMaskShader;
    const gfx::Renderable* clipMaskRenderable = nullptr;
    std::optional<std::size_t> clipMaskPipelineHash;
    std::vector<BufferResource> clipMaskUniformBuffers;
    std::vector<WGPUBindGroup> clipMaskActiveBindGroups;
};

} // namespace webgpu
} // namespace mbgl
