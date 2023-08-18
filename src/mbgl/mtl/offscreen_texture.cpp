#include <mbgl/mtl/offscreen_texture.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/texture2d.hpp>

#include <Metal/Metal.hpp>

namespace mbgl {
namespace mtl {

class OffscreenTextureResource final : public RenderableResource {
public:
    OffscreenTextureResource(Context& context_, const Size size_, const gfx::TextureChannelDataType type_)
        : context(context_),
          size(size_),
          type(type_) {
        assert(!size.isEmpty());
        texture = context.createTexture2D();
        texture->setSize(size);
        texture->setFormat(gfx::TexturePixelType::RGBA, type);
        texture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        static_cast<Texture2D*>(texture.get())
            ->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite | MTL::TextureUsageRenderTarget);
    }

    ~OffscreenTextureResource() noexcept override = default;

    void bind() override {
        assert(context.getBackend().getCommandQueue());
        commandBuffer = NS::RetainPtr(context.getBackend().getCommandQueue()->commandBuffer());
        texture->create();
    }

    void swap() override {
        assert(commandBuffer);
        commandBuffer->commit();
        commandBuffer->waitUntilCompleted();
        commandBuffer.reset();
    }

    PremultipliedImage readStillImage() {
        assert(false);
        return PremultipliedImage();
    }

    gfx::Texture2DPtr& getTexture() {
        assert(texture);
        return texture;
    }

    const RendererBackend& getBackend() const override { return context.getBackend(); }

    const MTLCommandBufferPtr& getCommandBuffer() const override { return commandBuffer; }

    MTLBlitPassDescriptorPtr getUploadPassDescriptor() const override {
        assert(false);
        return NS::TransferPtr(MTL::BlitPassDescriptor::alloc()->init());
    }

    MTLRenderPassDescriptorPtr getRenderPassDescriptor() const override {
        auto renderPassDescriptor = NS::TransferPtr(MTL::RenderPassDescriptor::alloc()->init());
        if (auto* colorTarget = renderPassDescriptor->colorAttachments()->object(0)) {
            colorTarget->setTexture(static_cast<Texture2D*>(texture.get())->getMetalTexture());
        }
        return renderPassDescriptor;
    }

private:
    Context& context;
    const Size size;
    const gfx::TextureChannelDataType type;
    gfx::Texture2DPtr texture;
    MTLCommandBufferPtr commandBuffer;
};

OffscreenTexture::OffscreenTexture(Context& context, const Size size_, const gfx::TextureChannelDataType type)
    : gfx::OffscreenTexture(size, std::make_unique<OffscreenTextureResource>(context, size_, type)) {}

bool OffscreenTexture::isRenderable() {
    assert(false);
    return true;
}

PremultipliedImage OffscreenTexture::readStillImage() {
    return getResource<OffscreenTextureResource>().readStillImage();
}

const gfx::Texture2DPtr& OffscreenTexture::getTexture() {
    return getResource<OffscreenTextureResource>().getTexture();
}

} // namespace mtl
} // namespace mbgl
