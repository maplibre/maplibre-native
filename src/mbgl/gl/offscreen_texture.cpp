#include <mbgl/gl/offscreen_texture.hpp>
#include <mbgl/gl/renderable_resource.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/framebuffer.hpp>
#include <mbgl/gfx/renderbuffer.hpp>

namespace mbgl {
namespace gl {

class OffscreenTextureResource final : public gl::RenderableResource {
public:
    OffscreenTextureResource(gl::Context& context_,
                             const Size size_,
                             const gfx::TextureChannelDataType type_,
                             const bool depth_,
                             const bool stencil_)
        : context(context_),
          size(size_),
          type(type_),
          depth(depth_),
          stencil(stencil_) {
        assert(!size.isEmpty());
        texture = context.createTexture2D();
        texture->setSize(size);
        texture->setFormat(gfx::TexturePixelType::RGBA, type);
        texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                          .wrapU = gfx::TextureWrapType::Clamp,
                                          .wrapV = gfx::TextureWrapType::Clamp});
    }

    ~OffscreenTextureResource() noexcept override = default;

    void bind() override {
        if (!framebuffer) {
            assert(texture);
            texture->create();
            if (stencil) {
                // Combined depth/stencil, as maplibre-gl-js attaches to its render-to-texture
                // framebuffer (createFramebuffer(w, h, /*depth*/ true, /*stencil*/ true)). The
                // stencil clips overlapping draped tiles against each other.
                depthStencilBuffer = context.createRenderbuffer<gfx::RenderbufferPixelType::DepthStencil>(size);
                framebuffer = context.createFramebuffer(*texture, *depthStencilBuffer);
            } else if (depth) {
                depthBuffer = context.createRenderbuffer<gfx::RenderbufferPixelType::Depth>(size);
                framebuffer = context.createFramebuffer(*texture, *depthBuffer);
            } else {
                framebuffer = context.createFramebuffer(*texture);
            }
        } else {
            context.bindFramebuffer = framebuffer->framebuffer;
        }

        context.activeTextureUnit = 0;
        context.scissorTest = {0, 0, 0, 0};
        context.viewport = {.x = 0, .y = 0, .size = size};
    }

    PremultipliedImage readStillImage() {
        assert(framebuffer);
        context.bindFramebuffer = framebuffer->framebuffer;
        return context.readFramebuffer<PremultipliedImage>(size);
    }

    gfx::Texture2DPtr& getTexture() {
        assert(texture);
        return texture;
    }

private:
    gl::Context& context;
    const Size size;
    gfx::Texture2DPtr texture;
    const gfx::TextureChannelDataType type;
    const bool depth;
    const bool stencil;
    std::optional<gfx::Renderbuffer<gfx::RenderbufferPixelType::Depth>> depthBuffer;
    std::optional<gfx::Renderbuffer<gfx::RenderbufferPixelType::DepthStencil>> depthStencilBuffer;
    std::optional<gl::Framebuffer> framebuffer;
};

OffscreenTexture::OffscreenTexture(gl::Context& context,
                                   const Size size_,
                                   const gfx::TextureChannelDataType type,
                                   const bool depth,
                                   const bool stencil)
    : gfx::OffscreenTexture(size, std::make_unique<OffscreenTextureResource>(context, size_, type, depth, stencil)) {}

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

const gfx::Texture2DPtr& OffscreenTexture::getTexture() {
    return getResource<OffscreenTextureResource>().getTexture();
}

} // namespace gl
} // namespace mbgl
