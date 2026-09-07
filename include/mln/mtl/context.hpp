#pragma once

#include <mln/gfx/draw_mode.hpp>
#include <mln/gfx/depth_mode.hpp>
#include <mln/gfx/stencil_mode.hpp>
#include <mln/gfx/color_mode.hpp>
#include <mln/gfx/texture2d.hpp>
#include <mln/gfx/context.hpp>
#include <mln/gfx/globe_clip_mask.hpp>
#include <mln/mtl/buffer_resource.hpp>
#include <mln/mtl/mtl_fwd.hpp>
#include <mln/mtl/uniform_buffer.hpp>
#include <mln/shaders/layer_ubo.hpp>
#include <mln/util/noncopyable.hpp>
#include <mln/util/containers.hpp>

#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <tuple>
#include <vector>

namespace mln {

class ProgramParameters;
class RenderStaticData;

namespace gfx {
class VertexAttributeArray;
using VertexAttributeArrayPtr = std::shared_ptr<VertexAttributeArray>;
} // namespace gfx

namespace shaders {
struct ClipUBO;
} // namespace shaders

namespace mtl {

class RenderPass;
class RendererBackend;
class ShaderProgram;
class VertexBufferResource;

using UniqueShaderProgram = std::unique_ptr<ShaderProgram>;
using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;
using UniqueUniformBufferArray = std::unique_ptr<gfx::UniformBufferArray>;

class Context final : public gfx::Context {
public:
    Context(RendererBackend&);
    ~Context() noexcept override;
    Context(const Context&) = delete;
    Context& operator=(const Context& other) = delete;

    const RendererBackend& getBackend() const { return backend; }

    void beginFrame() override;
    void endFrame() override;

    std::unique_ptr<gfx::CommandEncoder> createCommandEncoder() override;

    /// Create a new buffer object
    /// @param data The raw data to copy, may be `nullptr`
    /// @param size The size of the buffer
    /// @param usage Not currently used
    /// @param isIndexBuffer True if the buffer will be used for indexes.  The Metal API only accepts `MTLBuffer`
    /// objects for drawing indexed primitives, so this constrains how the buffer can be managed.
    /// @param persistent Performance hint, assume this buffer will be reused many times.
    BufferResource createBuffer(
        const void* data, std::size_t size, gfx::BufferUsageType usage, bool isIndexBuffer, bool persistent) const;

    UniqueShaderProgram createProgram(shaders::BuiltIn shaderID,
                                      std::string name,
                                      std::string_view source,
                                      std::string_view vertexName,
                                      std::string_view fragmentName,
                                      const ProgramParameters& programParameters,
                                      const mln::unordered_map<std::string, std::string>& additionalDefines);

    MTLTexturePtr createMetalTexture(MTLTextureDescriptorPtr textureDescriptor) const;
    MTLSamplerStatePtr createMetalSamplerState(MTLSamplerDescriptorPtr samplerDescriptor) const;

    /// Called at the end of a frame.
    void performCleanup() override;

    void reduceMemoryUsage() override {}

    gfx::UniqueDrawableBuilder createDrawableBuilder(std::string name) override;
    gfx::UniformBufferPtr createUniformBuffer(const void* data,
                                              std::size_t size,
                                              bool persistent = false,
                                              bool ssbo = false) override;

    UniqueUniformBufferArray createLayerUniformBufferArray() override;

    gfx::ShaderProgramBasePtr getGenericShader(gfx::ShaderRegistry&,
                                               const std::string& name,
                                               gfx::ProjectionVariant) override;

    TileLayerGroupPtr createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;

    LayerGroupPtr createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;

    gfx::Texture2DPtr createTexture2D() override;

    gfx::DynamicTexturePtr createDynamicTexture(Size size, gfx::TexturePixelType pixelType) override;

    RenderTargetPtr createRenderTarget(const Size size, const gfx::TextureChannelDataType type) override;

    void resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) override;

    void setDirtyState() override;

    void releaseGlobeClipMasks() override;

    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType, bool, bool);

    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType) override;

    std::unique_ptr<gfx::RenderbufferResource> createRenderbufferResource(gfx::RenderbufferPixelType,
                                                                          Size size) override;

    std::unique_ptr<gfx::DrawScopeResource> createDrawScopeResource() override;

    gfx::VertexAttributeArrayPtr createVertexAttributeArray() const override;

#if !defined(NDEBUG)
    void visualizeStencilBuffer() override;
    void visualizeDepthBuffer(float depthRangeSize) override;
#endif

    void clearStencilBuffer(int32_t) override;

    /// The depth-stencil state clip masks are drawn with, rebuilt when the renderable changes.
    const MTLDepthStencilStatePtr& clipMaskDepthStencilStateFor(const gfx::Renderable&);
    /// A pipeline state for a clip-mask shader over the position-only tile vertex layout.
    MTLRenderPipelineStatePtr makeClipMaskPipelineState(const ShaderProgram&, const gfx::Renderable&);
    /// The uniform buffer for one clip-mask pass: `persistent` on the first use in a frame, `temp` after that.
    std::optional<BufferResource>& clipMaskUniformBuffer(std::optional<BufferResource>& persistent,
                                                         bool& used,
                                                         std::optional<BufferResource>& temp,
                                                         const void* data,
                                                         std::size_t size);

    MTLDepthStencilStatePtr makeDepthStencilState(const gfx::DepthMode&,
                                                  const gfx::StencilMode&,
                                                  const gfx::Renderable&) const;

    bool emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr&,
                                      const void* data,
                                      std::size_t size,
                                      bool persistent) override;

    /// Get a reusable buffer containing the standard fixed tile vertices (+/- `util::EXTENT`)
    const BufferResource& getTileVertexBuffer();

    /// Get a reusable buffer containing the standard fixed tile indexes
    const BufferResource& getTileIndexBuffer();

    /// Get a buffer to be bound to unused vertex buffers
    const UniqueVertexBufferResource& getEmptyVertexBuffer();

    bool renderGlobeTileClippingMasks(gfx::RenderPass& renderPass,
                                      RenderStaticData& staticData,
                                      const std::vector<gfx::GlobeClipMask>& masks);

    bool renderTileClippingMasks(gfx::RenderPass& renderPass,
                                 RenderStaticData& staticData,
                                 const std::vector<shaders::ClipUBO>& tileUBOs);

    /// Get the global uniform buffers
    const gfx::UniformBufferArray& getGlobalUniformBuffers() const override { return globalUniformBuffers; };

    /// Get the mutable global uniform buffer array
    gfx::UniformBufferArray& mutableGlobalUniformBuffers() override { return globalUniformBuffers; };

    /// Bind the global uniform buffers
    void bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;

    /// Unbind the global uniform buffers
    void unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override {}

private:
    RendererBackend& backend;
    bool cleanupOnDestruction = true;

    std::optional<BufferResource> emptyBuffer;
    std::optional<BufferResource> tileVertexBuffer;
    std::optional<BufferResource> tileIndexBuffer;

    UniqueVertexBufferResource emptyVertexBuffer;

    gfx::ShaderProgramBasePtr clipMaskShader;
    MTLDepthStencilStatePtr clipMaskDepthStencilState;
    MTLRenderPipelineStatePtr clipMaskPipelineState;
    std::optional<BufferResource> clipMaskUniformsBuffer;
    bool clipMaskUniformsBufferUsed = false;

    struct GlobeClipMesh {
        BufferResource vertices;
        BufferResource indices;
        std::size_t indexCount;
    };
    gfx::ShaderProgramBasePtr globeClipMaskShader;
    MTLRenderPipelineStatePtr globeClipMaskPipelineState;
    std::optional<BufferResource> globeClipMaskUniformsBuffer;
    bool globeClipMaskUniformsBufferUsed = false;
    std::map<std::tuple<uint8_t, bool, bool>, GlobeClipMesh> globeClipMeshes;
    const gfx::Renderable* stencilStateRenderable = nullptr;

    UniformBufferArray globalUniformBuffers;
};

} // namespace mtl
} // namespace mln
