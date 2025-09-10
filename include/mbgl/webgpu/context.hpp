#pragma once

#include <mbgl/gfx/context.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace mbgl {

class ProgramParameters;

namespace webgpu {

class RendererBackend;
class ShaderProgram;
class BufferResource;
class Texture2D;

using UniqueShaderProgram = std::unique_ptr<ShaderProgram>;

class Context final : public gfx::Context {
public:
    explicit Context(RendererBackend& backend);
    ~Context() noexcept override;

    Context(const Context&) = delete;
    Context& operator=(const Context& other) = delete;

    RendererBackend& getBackend() const { return backend; }
    BackendImpl* getImpl() const { return impl.get(); }

    void beginFrame() override;
    void endFrame() override;

    void performCleanup() override;
    void reduceMemoryUsage() override;

    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType) override;
    std::unique_ptr<gfx::CommandEncoder> createCommandEncoder() override;

#if !defined(NDEBUG)
    void visualizeStencilBuffer() override {}
    void visualizeDepthBuffer(float) override {}
#endif

    void clearStencilBuffer(int32_t) override;
    void setDirtyState() override;

    gfx::VertexAttributeArrayPtr createVertexAttributeArray() const override;
    gfx::UniqueDrawableBuilder createDrawableBuilder(std::string name) override;
    
    gfx::UniformBufferPtr createUniformBuffer(const void* data,
                                             std::size_t size,
                                             bool persistent = false,
                                             bool ssbo = false) override;

    gfx::UniqueUniformBufferArray createLayerUniformBufferArray() override;
    gfx::ShaderProgramBasePtr getGenericShader(gfx::ShaderRegistry&, const std::string& name) override;

    TileLayerGroupPtr createTileLayerGroup(int32_t layerIndex,
                                          std::size_t initialCapacity,
                                          std::string name) override;

    LayerGroupPtr createLayerGroup(int32_t layerIndex,
                                  std::size_t initialCapacity,
                                  std::string name) override;

    gfx::Texture2DPtr createTexture2D() override;
    RenderTargetPtr createRenderTarget(const Size size, const gfx::TextureChannelDataType type) override;

    void resetState(gfx::DepthMode, gfx::ColorMode) override;

    bool emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr&,
                                     const void* data,
                                     std::size_t size,
                                     bool persistent = false) override;

    const gfx::UniformBufferArray& getGlobalUniformBuffers() const override;
    gfx::UniformBufferArray& mutableGlobalUniformBuffers() override;
    void bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;
    void unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;

    BufferResource createBuffer(const void* data, std::size_t size, std::uint32_t usage, bool persistent) const;

protected:
    std::unique_ptr<gfx::RenderbufferResource> createRenderbufferResource(gfx::RenderbufferPixelType, Size) override;
    std::unique_ptr<gfx::DrawScopeResource> createDrawScopeResource() override;

private:
    RendererBackend& backend;
    std::unique_ptr<BackendImpl> impl;
    gfx::UniformBufferArray globalUniformBuffers;
    bool framePending = false;
};

} // namespace webgpu
} // namespace mbgl