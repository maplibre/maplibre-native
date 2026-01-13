#pragma once

#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/scissor_rect.hpp>
#include <mbgl/gl/object.hpp>
#include <mbgl/gl/state.hpp>
#include <mbgl/gl/value.hpp>
#include <mbgl/gl/framebuffer.hpp>
#include <mbgl/gl/resource_pool.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/util/noncopyable.hpp>

#include <mbgl/gl/fence.hpp>
#include <mbgl/gl/buffer_allocator.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>

#include <array>
#include <functional>
#include <vector>

namespace mbgl {
namespace gl {

using ProcAddress = void (*)();
class RendererBackend;

namespace extension {
class VertexArray;
class Debugging;
} // namespace extension

class Context final : public gfx::Context {
public:
    Context(RendererBackend&);
    ~Context() noexcept override;
    Context(const Context&) = delete;
    Context& operator=(const Context& other) = delete;

    std::unique_ptr<gfx::CommandEncoder> createCommandEncoder() override;

    gfx::ContextObserver& getObserver() const { return *observer; }

    void beginFrame() override;
    void endFrame() override;

    void initializeExtensions(const std::function<gl::ProcAddress(const char*)>&);

    void enableDebugging();

    UniqueShader createShader(ShaderType type, const std::initializer_list<const char*>& sources);
    UniqueProgram createProgram(ShaderID vertexShader, ShaderID fragmentShader, const char* location0AttribName);
    void verifyProgramLinkage(ProgramID);
    void linkProgram(ProgramID);
    UniqueTexture createUniqueTexture(const Size& size, gfx::TexturePixelType format, gfx::TextureChannelDataType type);

    Framebuffer createFramebuffer(const gfx::Renderbuffer<gfx::RenderbufferPixelType::RGBA>&,
                                  const gfx::Renderbuffer<gfx::RenderbufferPixelType::DepthStencil>&);
    Framebuffer createFramebuffer(const gfx::Renderbuffer<gfx::RenderbufferPixelType::RGBA>&);

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
    void setScissorTest(const gfx::ScissorRect&);

    void draw(const gfx::DrawMode&, std::size_t indexOffset, std::size_t indexLength);

    void finish();

    std::shared_ptr<gl::Fence> getCurrentFrameFence() const;

    // Actually remove the objects we marked as abandoned with the above methods.
    // Only call this while the OpenGL context is exclusive to this thread.
    // Pooled textures are retained
    void performCleanup() override;

    // Called when the app receives a memory warning and before it goes to the background.
    // Calls performCleanup and destroy all pooled resources
    void reduceMemoryUsage() override;

    // Drain pools and remove abandoned objects, in preparation for destroying the store.
    // Only call this while the OpenGL context is exclusive to this thread.
    void reset();

    // Returns whether there are any objects that need to be cleaned up.
    // If considerPool is true, it will also check if resources are still pooled.
    bool empty(bool considerPool = false) const {
        return abandonedPrograms.empty() && abandonedShaders.empty() && abandonedBuffers.empty() &&
               abandonedTextures.empty() && abandonedVertexArrays.empty() && abandonedFramebuffers.empty() &&
               (considerPool ? texturePool->empty() : true);
    }

    extension::Debugging* getDebuggingExtension() const { return debugging.get(); }

    bool getCleanupOnDestruction() { return cleanupOnDestruction; }
    void setCleanupOnDestruction(bool cleanup) { cleanupOnDestruction = cleanup; }

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

    Framebuffer createFramebuffer(const gfx::Texture2D& color);

    gfx::VertexAttributeArrayPtr createVertexAttributeArray() const override;

    void resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) override;

    bool emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr&,
                                      const void* data,
                                      std::size_t size,
                                      bool persistent) override;

    /// Get the global uniform buffers
    const gfx::UniformBufferArray& getGlobalUniformBuffers() const override { return globalUniformBuffers; };

    /// Get the mutable global uniform buffer array
    gfx::UniformBufferArray& mutableGlobalUniformBuffers() override { return globalUniformBuffers; };

    /// Bind the global uniform buffers
    void bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;

    /// Unbind the global uniform buffers
    void unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;

    void setDirtyState() override;

    Texture2DPool& getTexturePool();

private:
    RendererBackend& backend;
    bool cleanupOnDestruction = true;

    std::unique_ptr<extension::Debugging> debugging;
    std::shared_ptr<gl::Fence> frameInFlightFence;
    std::unique_ptr<gl::UniformBufferAllocator> uboAllocator;
    size_t frameNum = 0;
    UniformBufferArrayGL globalUniformBuffers;

public:
    State<value::ActiveTextureUnit> activeTextureUnit;
    State<value::BindFramebuffer> bindFramebuffer;
    State<value::Viewport> viewport;
    State<value::ScissorTest> scissorTest;
    std::array<State<value::BindTexture>, gfx::MaxActiveTextureUnits> texture;
    State<value::Program> program;
    State<value::BindVertexBuffer> vertexBuffer;

    State<value::BindVertexArray> bindVertexArray;
    VertexArrayState globalVertexArrayState{UniqueVertexArray(0, {const_cast<Context*>(this)})};

    State<value::PixelStorePack> pixelStorePack;
    State<value::PixelStoreUnpack> pixelStoreUnpack;

private:
    State<value::StencilFunc> stencilFunc;
    State<value::StencilMask> stencilMask;
    State<value::StencilTest> stencilTest;
    State<value::StencilOp> stencilOp;
#if MLN_RENDER_BACKEND_OPENGL
    State<value::DepthRange> depthRange;
#endif
    State<value::DepthMask> depthMask;
    State<value::DepthTest> depthTest;
    State<value::DepthFunc> depthFunc;
    State<value::Blend> blend;
    State<value::BlendEquation> blendEquation;
    State<value::BlendFunc> blendFunc;
    State<value::BlendColor> blendColor;
    State<value::ColorMask> colorMask;
    State<value::ClearDepth> clearDepth;
    State<value::ClearColor> clearColor;
    State<value::ClearStencil> clearStencil;
    State<value::LineWidth> lineWidth;
    State<value::BindRenderbuffer> bindRenderbuffer;
    State<value::CullFace> cullFace;
    State<value::CullFaceSide> cullFaceSide;
    State<value::CullFaceWinding> cullFaceWinding;

public:
    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType) override;

private:
    std::unique_ptr<gfx::RenderbufferResource> createRenderbufferResource(gfx::RenderbufferPixelType,
                                                                          Size size) override;

    std::unique_ptr<gfx::DrawScopeResource> createDrawScopeResource() override;

    UniqueFramebuffer createFramebuffer();
    std::unique_ptr<uint8_t[]> readFramebuffer(Size, gfx::TexturePixelType, bool flip);

public:
    VertexArray createVertexArray();

private:
    friend detail::ProgramDeleter;
    friend detail::ShaderDeleter;
    friend detail::BufferDeleter;
    friend detail::TextureDeleter;
    friend detail::VertexArrayDeleter;
    friend detail::FramebufferDeleter;
    friend detail::RenderbufferDeleter;

    std::vector<ProgramID> abandonedPrograms;
    std::vector<ShaderID> abandonedShaders;
    std::vector<BufferID> abandonedBuffers;
    std::vector<TextureID> abandonedTextures;
    std::vector<VertexArrayID> abandonedVertexArrays;
    std::vector<FramebufferID> abandonedFramebuffers;
    std::vector<RenderbufferID> abandonedRenderbuffers;

    std::unique_ptr<Texture2DPool> texturePool;

public:
#if !defined(NDEBUG)
public:
    void visualizeStencilBuffer() override;
    void visualizeDepthBuffer(float depthRangeSize) override;
#endif

    void clearStencilBuffer(int32_t) override;
};

} // namespace gl
} // namespace mbgl
