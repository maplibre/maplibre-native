#include <mbgl/vulkan/offscreen_texture.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/texture2d.hpp>

namespace mbgl {
namespace vulkan {

class OffscreenTextureResource final : public RenderableResource {
public:
    OffscreenTextureResource(
        Context& context_, const Size size_, const gfx::TextureChannelDataType type_, bool depth, bool stencil)
        : context(context_),
          size(size_),
          type(type_) {
        assert(!size.isEmpty());
        colorTexture = context.createTexture2D();
        colorTexture->setSize(size);
        colorTexture->setFormat(gfx::TexturePixelType::RGBA, type);
        colorTexture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});

        if (depth) {
            depthTexture = context.createTexture2D();
            depthTexture->setSize(size);
            depthTexture->setFormat(gfx::TexturePixelType::Depth, gfx::TextureChannelDataType::Float);
            depthTexture->setSamplerConfiguration(
                {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        }

        context.renderingStats().numFrameBuffers++;
    }

    ~OffscreenTextureResource() noexcept override { context.renderingStats().numFrameBuffers--; }

    void bind() override { colorTexture->create(); }

    void swap() override {}

    PremultipliedImage readStillImage() { return {}; }

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

OffscreenTexture::OffscreenTexture(
    Context& context, const Size size_, const gfx::TextureChannelDataType type, bool depth, bool stencil)
    : gfx::OffscreenTexture(size, std::make_unique<OffscreenTextureResource>(context, size_, type, depth, stencil)) {}

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

} // namespace vulkan
} // namespace mbgl
