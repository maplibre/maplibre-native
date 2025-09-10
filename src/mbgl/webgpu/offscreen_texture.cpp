#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/texture2d.hpp>

namespace mbgl {
namespace webgpu {

class OffscreenTextureResource final : public RenderableResource {
public:
    OffscreenTextureResource(Context& context_,
                             const Size size_,
                             const gfx::TextureChannelDataType type_,
                             bool depth,
                             bool stencil)
        : context(context_),
          size(size_),
          type(type_) {
        assert(!size.isEmpty());
        
        // Create color texture
        colorTexture = context.createTexture2D();
        colorTexture->setSize(size);
        colorTexture->setFormat(gfx::TexturePixelType::RGBA, type);
        colorTexture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        
        // Create depth texture if needed
        if (depth) {
            depthTexture = context.createTexture2D();
            depthTexture->setSize(size);
            depthTexture->setFormat(gfx::TexturePixelType::Depth, gfx::TextureChannelDataType::Float);
            depthTexture->setSamplerConfiguration(
                {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        }
        
        // Create stencil texture if needed
        if (stencil) {
            stencilTexture = context.createTexture2D();
            stencilTexture->setSize(size);
            stencilTexture->setFormat(gfx::TexturePixelType::Stencil, gfx::TextureChannelDataType::UnsignedByte);
            stencilTexture->setSamplerConfiguration(
                {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        }
    }
    
    ~OffscreenTextureResource() noexcept override {
        context.renderingStats().numFrameBuffers--;
    }
    
    void bind() override {
        // Create textures if not already created
        colorTexture->create();
        
        if (depthTexture) {
            depthTexture->create();
        }
        
        if (stencilTexture) {
            stencilTexture->create();
        }
    }
    
    void swap() override {
        // WebGPU doesn't require explicit swap for offscreen textures
    }
    
    const RendererBackend& getBackend() const override {
        return context.getBackend();
    }
    
    PremultipliedImage readStillImage() {
        // Read back the color texture data
        auto data = std::make_unique<uint8_t[]>(colorTexture->getDataSize());
        // TODO: Implement WebGPU texture readback
        return {size, std::move(data)};
    }
    
    gfx::Texture2DPtr& getTexture() {
        assert(colorTexture);
        return colorTexture;
    }
    
private:
    Context& context;
    const Size size;
    const gfx::TextureChannelDataType type;
    gfx::Texture2DPtr colorTexture;
    gfx::Texture2DPtr depthTexture;
    gfx::Texture2DPtr stencilTexture;
};

OffscreenTexture::OffscreenTexture(Context& context,
                                   const Size size_,
                                   const gfx::TextureChannelDataType type,
                                   bool depth,
                                   bool stencil)
    : gfx::OffscreenTexture(size_, std::make_unique<OffscreenTextureResource>(context, size_, type, depth, stencil)) {
}

bool OffscreenTexture::isRenderable() {
    // TODO: Implement renderability check
    return true;
}

PremultipliedImage OffscreenTexture::readStillImage() {
    return getResource<OffscreenTextureResource>().readStillImage();
}

const gfx::Texture2DPtr& OffscreenTexture::getTexture() {
    return getResource<OffscreenTextureResource>().getTexture();
}

} // namespace webgpu
} // namespace mbgl