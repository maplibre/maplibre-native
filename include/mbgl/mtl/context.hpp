#pragma once

#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/mtl/buffer_resource.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>
#include <mbgl/util/noncopyable.hpp>

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace mbgl {

class ProgramParameters;
class RenderStaticData;

namespace shaders {
struct ClipUBO;
} // namespace shaders

namespace mtl {

class RenderPass;
class RendererBackend;
class ShaderProgram;

using UniqueShaderProgram = std::unique_ptr<ShaderProgram>;

class Context final : public gfx::Context {
public:
    Context(RendererBackend&);
    ~Context() noexcept override;
    Context(const Context&) = delete;
    Context& operator=(const Context& other) = delete;

    const RendererBackend& getBackend() const { return backend; }

    std::unique_ptr<gfx::CommandEncoder> createCommandEncoder() override;

    gfx::RenderingStats& renderingStats() { return stats; }
    const gfx::RenderingStats& renderingStats() const override { return stats; }

    BufferResource createBuffer(const void* data, std::size_t size, gfx::BufferUsageType) const;

    UniqueShaderProgram createProgram(std::string name,
                                      std::string_view source,
                                      std::string_view vertexName,
                                      std::string_view fragmentName,
                                      const ProgramParameters& programParameters,
                                      const std::unordered_map<std::string, std::string>& additionalDefines);

    MTLTexturePtr createMetalTexture(MTLTextureDescriptorPtr textureDescriptor) const;
    MTLSamplerStatePtr createMetalSamplerState(MTLSamplerDescriptorPtr samplerDescriptor) const;

    // Actually remove the objects we marked as abandoned with the above methods.
    void performCleanup() override;

    void reduceMemoryUsage() override {}

    gfx::UniqueDrawableBuilder createDrawableBuilder(std::string name) override;
    gfx::UniformBufferPtr createUniformBuffer(const void* data, std::size_t size) override;

    gfx::ShaderProgramBasePtr getGenericShader(gfx::ShaderRegistry&, const std::string& name) override;

    TileLayerGroupPtr createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;

    LayerGroupPtr createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;

    gfx::Texture2DPtr createTexture2D() override;

    RenderTargetPtr createRenderTarget(const Size size, const gfx::TextureChannelDataType type) override;

    // UniqueFramebuffer createFramebuffer(const gfx::Texture2D& color);

    void resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) override;

    void setDirtyState() override;

    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType, bool, bool);

    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType) override;

    std::unique_ptr<gfx::TextureResource> createTextureResource(Size,
                                                                gfx::TexturePixelType,
                                                                gfx::TextureChannelDataType) override;

    std::unique_ptr<gfx::RenderbufferResource> createRenderbufferResource(gfx::RenderbufferPixelType,
                                                                          Size size) override;

    std::unique_ptr<gfx::DrawScopeResource> createDrawScopeResource() override;
    /*
         UniqueFramebuffer createFramebuffer();
         std::unique_ptr<uint8_t[]> readFramebuffer(Size, gfx::TexturePixelType, bool flip);
    */

#if !defined(NDEBUG)
    void visualizeStencilBuffer() override;
    void visualizeDepthBuffer(float depthRangeSize) override;
#endif

    void clearStencilBuffer(int32_t) override;

    MTLDepthStencilStatePtr makeDepthStencilState(const gfx::DepthMode&,
                                                  const gfx::StencilMode&,
                                                  const gfx::Renderable&) const;

    virtual bool emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr&, const void* data, std::size_t size);

    /// Get a reusable buffer containing the standard fixed tile vertices (+/- `util::EXTENT`)
    const BufferResource& getTileVertexBuffer();

    /// Get a reusable buffer containing the standard fixed tile indexes
    const BufferResource& getTileIndexBuffer();

    bool renderTileClippingMasks(gfx::RenderPass& renderPass,
                                 RenderStaticData& staticData,
                                 const std::vector<shaders::ClipUBO>& tileUBOs);

private:
    RendererBackend& backend;
    bool cleanupOnDestruction = true;

    std::optional<BufferResource> tileVertexBuffer;
    std::optional<BufferResource> tileIndexBuffer;

    gfx::ShaderProgramBasePtr clipMaskShader;
    MTLDepthStencilStatePtr clipMaskDepthStencilState;
    MTLRenderPipelineStatePtr clipMaskPipelineState;
    BufferResource clipMaskUniformsBuffer;
    bool clipMaskUniformsBufferUsed = false;
    const gfx::Renderable* stencilStateRenderable = nullptr;

    gfx::RenderingStats stats;
};

} // namespace mtl
} // namespace mbgl
