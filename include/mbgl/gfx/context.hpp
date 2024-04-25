#pragma once

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/gfx/backend.hpp>
#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/gfx/draw_scope.hpp>
#include <mbgl/gfx/program.hpp>
#include <mbgl/gfx/renderbuffer.hpp>
#include <mbgl/gfx/rendering_stats.hpp>
#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/types.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/uniform_buffer.hpp>
#endif

#include <memory>
#include <string>

namespace mbgl {

class PaintParameters;
class ProgramParameters;

#if MLN_DRAWABLE_RENDERER
class TileLayerGroup;
class LayerGroup;
class RenderTarget;
using TileLayerGroupPtr = std::shared_ptr<TileLayerGroup>;
using LayerGroupPtr = std::shared_ptr<LayerGroup>;
using RenderTargetPtr = std::shared_ptr<RenderTarget>;
#endif

namespace gfx {

class OffscreenTexture;
class ShaderRegistry;

#if MLN_DRAWABLE_RENDERER
class Drawable;
class DrawableBuilder;
class ShaderProgramBase;
class Texture2D;
class VertexAttributeArray;

using DrawablePtr = std::shared_ptr<Drawable>;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;
using Texture2DPtr = std::shared_ptr<Texture2D>;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
using UniqueDrawableBuilder = std::unique_ptr<DrawableBuilder>;
using VertexAttributeArrayPtr = std::shared_ptr<VertexAttributeArray>;
#endif

class Context {
protected:
    Context(uint32_t maximumVertexBindingCount_)
        : maximumVertexBindingCount(maximumVertexBindingCount_),
          backgroundScheduler(Scheduler::GetBackground()) {}

public:
    static constexpr const uint32_t minimumRequiredVertexBindingCount = 8;
    const uint32_t maximumVertexBindingCount;

    Context(Context&&) = delete;
    Context(const Context&) = delete;
    Context& operator=(Context&& other) = delete;
    Context& operator=(const Context& other) = delete;
    virtual ~Context() = default;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    /// Called at the end of a frame.
    virtual void performCleanup() = 0;

    /// Called when the app receives a memory warning and before it goes to the background.
    virtual void reduceMemoryUsage() = 0;

    virtual std::unique_ptr<OffscreenTexture> createOffscreenTexture(Size, TextureChannelDataType) = 0;

    /// Creates an empty texture with the specified dimensions.
    Texture createTexture(const Size size,
                          TexturePixelType format = TexturePixelType::RGBA,
                          TextureChannelDataType type = TextureChannelDataType::UnsignedByte) {
        return {size, createTextureResource(size, format, type)};
    }

    template <RenderbufferPixelType pixelType>
    Renderbuffer<pixelType> createRenderbuffer(const Size size) {
        return {size, createRenderbufferResource(pixelType, size)};
    }

    DrawScope createDrawScope() { return DrawScope{createDrawScopeResource()}; }

    virtual std::unique_ptr<CommandEncoder> createCommandEncoder() = 0;

    gfx::RenderingStats& renderingStats() { return stats; }
    const gfx::RenderingStats& renderingStats() const { return stats; }

#if !defined(NDEBUG)
    virtual void visualizeStencilBuffer() = 0;
    virtual void visualizeDepthBuffer(float depthRangeSize) = 0;
#endif

    virtual void clearStencilBuffer(int32_t) = 0;

    /// Sets dirty state
    virtual void setDirtyState() = 0;

#if MLN_DRAWABLE_RENDERER
    /// Create a new vertex attribute array
    virtual gfx::VertexAttributeArrayPtr createVertexAttributeArray() const = 0;

    /// Create a new drawable builder
    virtual UniqueDrawableBuilder createDrawableBuilder(std::string name) = 0;

    /// Create a new uniform buffer
    /// @param data The data to copy, may be `nullptr`
    /// @param size The size of the buffer
    /// @param persistent Performance hint, optimize for few or many uses
    virtual UniformBufferPtr createUniformBuffer(const void* data, std::size_t size, bool persistent = false) = 0;

    /// Get the generic shader with the specified name
    virtual gfx::ShaderProgramBasePtr getGenericShader(gfx::ShaderRegistry&, const std::string& name) = 0;

    /// Create a tile layer group implementation
    virtual TileLayerGroupPtr createTileLayerGroup(int32_t layerIndex,
                                                   std::size_t initialCapacity,
                                                   std::string name) = 0;

    /// Create a layer group implementation
    virtual LayerGroupPtr createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) = 0;

    /// Create a texture
    virtual Texture2DPtr createTexture2D() = 0;

    /// Create a render target
    virtual RenderTargetPtr createRenderTarget(const Size size, const TextureChannelDataType type) = 0;

    /// Resets the context state to defaults
    virtual void resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) = 0;

    /// Update the uniform buffer with the provided data if it already exists, otherwise create it.
    ///  @return True if the buffer was created, false if it was updated
    virtual bool emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr&,
                                              const void* data,
                                              std::size_t size,
                                              bool persistent = false) = 0;

    /// `emplaceOrUpdateUniformBuffer` with type inference
    template <typename T>
    std::enable_if_t<!std::is_pointer_v<T>, bool> emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr& ptr,
                                                                               const T* data,
                                                                               bool persistent = false) {
        return emplaceOrUpdateUniformBuffer(ptr, data, sizeof(T), persistent);
    }

    /// Get the global uniform buffers
    virtual const gfx::UniformBufferArray& getGlobalUniformBuffers() const = 0;

    /// Get the mutable global uniform buffer array
    virtual gfx::UniformBufferArray& mutableGlobalUniformBuffers() = 0;

    /// Bind the global uniform buffers
    virtual void bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept = 0;

    /// Unbind the global uniform buffers
    virtual void unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept = 0;
#endif

protected:
    virtual std::unique_ptr<TextureResource> createTextureResource(Size, TexturePixelType, TextureChannelDataType) = 0;
    virtual std::unique_ptr<RenderbufferResource> createRenderbufferResource(RenderbufferPixelType, Size) = 0;
    virtual std::unique_ptr<DrawScopeResource> createDrawScopeResource() = 0;

    gfx::RenderingStats stats;

    std::shared_ptr<Scheduler> backgroundScheduler;
};

} // namespace gfx
} // namespace mbgl
