#include <mbgl/gl/offscreen_texture.hpp>
#include <mbgl/gl/renderable_resource.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/framebuffer.hpp>

namespace mbgl {
namespace gl {

class OffscreenTextureResource final : public gl::RenderableResource {
public:
    OffscreenTextureResource(gl::Context& context_, const Size size_, const gfx::TextureChannelDataType type_)
        : context(context_),
          size(size_),
          type(type_) {
        assert(!size.isEmpty());
#if MLN_DRAWABLE_RENDERER
        texture = context.createTexture2D();
        texture->setSize(size);
        texture->setFormat(gfx::TexturePixelType::RGBA, type);
        texture->setSamplerConfiguration(
            {gfx::TextureFilterType::Nearest, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
#endif
    }

    ~OffscreenTextureResource() noexcept override = default;

    void bind() override {
        if (!framebuffer) {
#if MLN_LEGACY_RENDERER
            assert(!texture);
            texture = context.createTexture(size, gfx::TexturePixelType::RGBA, type);
            framebuffer = context.createFramebuffer(*texture);
#else
            assert(texture);
            texture->create();
            framebuffer = context.createFramebuffer(*texture);
#endif
        } else {
            context.bindFramebuffer = framebuffer->framebuffer;
        }

        context.activeTextureUnit = 0;
        context.scissorTest = false;
        context.viewport = {0, 0, size};
    }

    PremultipliedImage readStillImage() {
        assert(framebuffer);
        context.bindFramebuffer = framebuffer->framebuffer;
        return context.readFramebuffer<PremultipliedImage>(size);
    }

#if MLN_LEGACY_RENDERER
    gfx::Texture& getTexture() {
        assert(texture);
        return *texture;
    }
#else
    gfx::Texture2DPtr& getTexture() {
        assert(texture);
        return texture;
    }
#endif

private:
    gl::Context& context;
    const Size size;
#if MLN_LEGACY_RENDERER
    std::optional<gfx::Texture> texture;
#else
    gfx::Texture2DPtr texture;
#endif
    const gfx::TextureChannelDataType type;
    std::optional<gl::Framebuffer> framebuffer;
};

OffscreenTexture::OffscreenTexture(gl::Context& context, const Size size_, const gfx::TextureChannelDataType type)
    : gfx::OffscreenTexture(size, std::make_unique<OffscreenTextureResource>(context, size_, type)) {}

bool OffscreenTexture::isRenderable() {
    try {
        getResource<OffscreenTextureResource>().bind();
        return true;
    } catch (const std::runtime_error&) {
        return false;
    }
}

PremultipliedImage OffscreenTexture::readStillImage() {
    return getResource<OffscreenTextureResource>().readStillImage();
}

#if MLN_LEGACY_RENDERER
gfx::Texture& OffscreenTexture::getTexture() {
    return getResource<OffscreenTextureResource>().getTexture();
}
#else
const gfx::Texture2DPtr& OffscreenTexture::getTexture() {
    return getResource<OffscreenTextureResource>().getTexture();
}
#endif

} // namespace gl
} // namespace mbgl
