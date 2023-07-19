#pragma once

#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/util/noncopyable.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/texture2d.hpp>
#endif

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

    const gfx::RenderingStats& renderingStats() const override { return stats; }

    UniqueShaderProgram createProgram(std::string name,
                                      std::string_view source,
                                      std::string_view vertexName,
                                      std::string_view fragmentName,
                                      const ProgramParameters& programParameters,
                                      const std::unordered_map<std::string, std::string>& additionalDefines);

    /*
    void verifyProgramLinkage(ProgramID);
    void linkProgram(ProgramID);
    UniqueTexture createUniqueTexture();

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

    void clear(std::optional<mbgl::Color> color, std::optional<float> depth, std::optional<int32_t> stencil);

    void setDepthMode(const gfx::DepthMode&);
    void setStencilMode(const gfx::StencilMode&);
    void setColorMode(const gfx::ColorMode&);
    void setCullFaceMode(const gfx::CullFaceMode&);

    void draw(const gfx::DrawMode&, std::size_t indexOffset, std::size_t indexLength);

    void finish();
*/

    // Actually remove the objects we marked as abandoned with the above methods.
    // Only call this while the OpenGL context is exclusive to this thread.
    void performCleanup() override;

    void reduceMemoryUsage() override;

/*
    // Drain pools and remove abandoned objects, in preparation for destroying the store.
    // Only call this while the OpenGL context is exclusive to this thread.
    void reset();

    bool empty() const {
        return pooledTextures.empty() && abandonedPrograms.empty() && abandonedShaders.empty() &&
               abandonedBuffers.empty() && abandonedTextures.empty() && abandonedVertexArrays.empty() &&
               abandonedFramebuffers.empty();
    }

    extension::Debugging* getDebuggingExtension() const { return debugging.get(); }

    void setCleanupOnDestruction(bool cleanup) { cleanupOnDestruction = cleanup; }
*/
    
#if MLN_DRAWABLE_RENDERER
    gfx::UniqueDrawableBuilder createDrawableBuilder(std::string name) override;
    gfx::UniformBufferPtr createUniformBuffer(const void* data, std::size_t size) override;

    gfx::ShaderProgramBasePtr getGenericShader(gfx::ShaderRegistry&, const std::string& name) override;

    TileLayerGroupPtr createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;

    LayerGroupPtr createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;

    gfx::Texture2DPtr createTexture2D() override;

    RenderTargetPtr createRenderTarget(const Size size, const gfx::TextureChannelDataType type) override;

    //UniqueFramebuffer createFramebuffer(const gfx::Texture2D& color);

    void resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) override;
#endif

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

     VertexArray createVertexArray();
*/
#if !defined(NDEBUG)
     void visualizeStencilBuffer() override;
     void visualizeDepthBuffer(float depthRangeSize) override;
#endif

     void clearStencilBuffer(int32_t) override;

private:
    RendererBackend& backend;
    bool cleanupOnDestruction = true;

    gfx::RenderingStats stats;
    
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace gl
} // namespace mbgl

