#pragma once

#include <mbgl/gfx/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <memory>

namespace mbgl {
namespace webgpu {

class Context : public gfx::Context {
public:
    explicit Context(RendererBackend& backend);
    ~Context() override;

    // Frame management
    void beginFrame() override;
    void endFrame() override;
    void performCleanup() override;
    void reduceMemoryUsage() override;

    // Resource creation
    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType) override;
    std::unique_ptr<gfx::CommandEncoder> createCommandEncoder() override;
    gfx::VertexAttributeArrayPtr createVertexAttributeArray() const override;
    gfx::UniqueDrawableBuilder createDrawableBuilder(std::string name) override;
    gfx::UniformBufferPtr createUniformBuffer(const void* data, std::size_t size, bool persistent = false, bool ssbo = false) override;
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
    bool emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr&, const void* data, std::size_t size, bool persistent = false) override;
    const gfx::UniformBufferArray& getGlobalUniformBuffers() const override;
    gfx::UniformBufferArray& mutableGlobalUniformBuffers() override;
    void bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;
    void unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;

#if !defined(NDEBUG)
    void visualizeStencilBuffer() override;
    void visualizeDepthBuffer(float depthRangeSize) override;
#endif

    // WebGPU specific
    class Impl;
    Impl* getImpl() const { return impl.get(); }

protected:
    std::unique_ptr<gfx::RenderbufferResource> createRenderbufferResource(gfx::RenderbufferPixelType, Size) override;
    std::unique_ptr<gfx::DrawScopeResource> createDrawScopeResource() override;

private:
    std::unique_ptr<Impl> impl;
    RendererBackend& backend;
    std::unique_ptr<gfx::UniformBufferArray> globalUniformBuffers;
};

} // namespace webgpu
} // namespace mbgl