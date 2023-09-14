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
#include <unordered_map>

namespace mbgl {

class ProgramParameters;

namespace mtl {

class RendererBackend;
class ShaderProgram;

using UniqueShaderProgram = std::unique_ptr<ShaderProgram>;

class Context final : public gfx::Context {
public:
    Context(RendererBackend&);
    ~Context() noexcept override;
    Context(const Context&) = delete;
    Context& operator=(const Context& other) = delete;

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

    /*
    Framebuffer createFramebuffer(const gfx::Renderbuffer<gfx::RenderbufferPixelType::RGBA>&,
                                  const gfx::Renderbuffer<gfx::RenderbufferPixelType::DepthStencil>&);
    Framebuffer createFramebuffer(const gfx::Renderbuffer<gfx::RenderbufferPixelType::RGBA>&);
    Framebuffer createFramebuffer(const gfx::Texture&,
                                  const gfx::Renderbuffer<gfx::RenderbufferPixelType::DepthStencil>&);
    Framebuffer createFramebuffer(const gfx::Texture&);
    Framebuffer createFramebuffer(const gfx::Texture&, const gfx::Renderbuffer<gfx::RenderbufferPixelType::Depth>&);

    template <typename Image,
              gfx::TexturePixelType format = Image::channels == 4 ? gfx::TexturePixelType::RGBA
                                                                  : gfx::TexturePixelType::Alpha>
    Image readFramebuffer(const Size size, bool flip = true) {
        static_assert(Image::channels == (format == gfx::TexturePixelType::RGBA ? 4 : 1), "image format mismatch");
        return {size, readFramebuffer(size, format, flip)};
    }
*/

    // Actually remove the objects we marked as abandoned with the above methods.
    void performCleanup() override {}

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

    virtual bool emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr&, const void* data, std::size_t size);

private:
    RendererBackend& backend;
    bool cleanupOnDestruction = true;

    gfx::RenderingStats stats;
};

} // namespace mtl
} // namespace mbgl
