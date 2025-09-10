// WebGPU implementation file with all the smaller classes

#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/webgpu/renderbuffer.hpp>
#include <mbgl/webgpu/vertex_attribute_array.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/render_target.hpp>
#include <mbgl/webgpu/tile_layer_group.hpp>
#include <mbgl/webgpu/layer_group.hpp>
#include <mbgl/webgpu/uniform_buffer_array.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gfx/render_target.hpp>
#include <mbgl/gfx/tile_layer_group.hpp>
#include <mbgl/gfx/layer_group.hpp>
#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/gfx/draw_scope.hpp>
#include <mbgl/gfx/renderbuffer.hpp>

namespace mbgl {
namespace webgpu {

// CommandEncoder
class CommandEncoder : public gfx::CommandEncoder {
public:
    explicit CommandEncoder(Context& context) : context_(context) {}
    ~CommandEncoder() override = default;
    
    void setDepthMode(const gfx::DepthMode&) override {}
    void setStencilMode(const gfx::StencilMode&) override {}
    void setColorMode(const gfx::ColorMode&) override {}
    void setCullFaceMode(const gfx::CullFaceMode&) override {}
    void setDrawableProvider(gfx::DrawableProvider*) override {}
    void setScissorTest(bool) override {}
    void setScissorRect(const gfx::ScissorTest&) override {}
    void clearColor(const Color&) override {}
    void clearStencil(int32_t) override {}
    void clearDepth(float) override {}
    void setViewport(const gfx::Viewport&) override {}
    void draw(const gfx::DrawablePtr&) override {}
    
    std::unique_ptr<gfx::RenderPass> createRenderPass(const char*, const gfx::RenderPassDescriptor&) override {
        return nullptr;
    }
    
    void present(gfx::Renderable&) override {}
    void pushDebugGroup(const char*) override {}
    void popDebugGroup() override {}
    
private:
    Context& context_;
};

// OffscreenTexture
class OffscreenTexture : public gfx::OffscreenTexture {
public:
    OffscreenTexture(Context& context, Size size) 
        : gfx::OffscreenTexture(size, gfx::TextureChannelDataType::UnsignedByte) {}
    ~OffscreenTexture() override = default;
    
    gfx::Texture2D& getTexture() override { return *texture; }
    gfx::RenderTarget& getRenderTarget() override { return *renderTarget; }
    bool isRenderable() const override { return true; }
    
private:
    std::unique_ptr<gfx::Texture2D> texture;
    std::unique_ptr<gfx::RenderTarget> renderTarget;
};

// VertexAttributeArray
class VertexAttributeArray : public gfx::VertexAttributeArray {
public:
    VertexAttributeArray() = default;
    ~VertexAttributeArray() override = default;
};

// Texture2D
class Texture2D : public gfx::Texture2D {
public:
    explicit Texture2D(Context& context) {}
    ~Texture2D() override = default;
    
    Size getSize() const override { return {256, 256}; }
    size_t getDataSize() const override { return 256 * 256 * 4; }
    size_t numChannels() const override { return 4; }
    void upload(const void* data, Size size, gfx::TexturePixelType, gfx::TextureChannelDataType) override {}
    void uploadSubRegion(const void* data, Size size, uint16_t x, uint16_t y, gfx::TexturePixelType, gfx::TextureChannelDataType) override {}
    void setSamplerConfiguration(const gfx::SamplerConfiguration&) noexcept override {}
    gfx::SamplerConfiguration getSamplerConfiguration() const noexcept override { return {}; }
};

// RenderTarget
class RenderTarget : public gfx::RenderTarget {
public:
    RenderTarget(Size size, gfx::TextureChannelDataType type) : size_(size) {}
    ~RenderTarget() override = default;
    Size getSize() const override { return size_; }
    
private:
    Size size_;
};

// TileLayerGroup
class TileLayerGroup : public gfx::TileLayerGroup {
public:
    TileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) 
        : gfx::TileLayerGroup(layerIndex, initialCapacity, std::move(name)) {}
    ~TileLayerGroup() override = default;
    
    void upload(gfx::UploadPass&) override {}
    void render(gfx::RenderPass&, const gfx::RenderTree&, PaintParameters&) override {}
    const gfx::UniformBufferArray& getUniformBuffers() const override { 
        static gfx::UniformBufferArray empty;
        return empty;
    }
    gfx::UniformBufferArray& mutableUniformBuffers() override {
        static gfx::UniformBufferArray empty;
        return empty;
    }
};

// LayerGroup
class LayerGroup : public gfx::LayerGroup {
public:
    LayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name)
        : gfx::LayerGroup(layerIndex, initialCapacity, std::move(name)) {}
    ~LayerGroup() override = default;
    
    void upload(gfx::UploadPass&) override {}
    void render(gfx::RenderPass&, const gfx::RenderTree&, PaintParameters&) override {}
    const gfx::UniformBufferArray& getUniformBuffers() const override {
        static gfx::UniformBufferArray empty;
        return empty;
    }
    gfx::UniformBufferArray& mutableUniformBuffers() override {
        static gfx::UniformBufferArray empty;
        return empty;
    }
};

// UniformBufferArray
class UniformBufferArray : public gfx::UniformBufferArray {
public:
    UniformBufferArray() = default;
    ~UniformBufferArray() override = default;
    
    std::size_t getAllocatedSize() const override { return 0; }
    std::size_t getAllocatedCount() const override { return 0; }
    bool isEmpty() const override { return true; }
};

// RenderbufferResource
class RenderbufferResource : public gfx::RenderbufferResource {
public:
    RenderbufferResource() = default;
    ~RenderbufferResource() override = default;
};

// DrawScopeResource  
class DrawScopeResource : public gfx::DrawScopeResource {
public:
    explicit DrawScopeResource(Context& context) {}
    ~DrawScopeResource() override = default;
    void bind() const override {}
};

} // namespace webgpu
} // namespace mbgl