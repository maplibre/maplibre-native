#include <mbgl/vulkan/offscreen_texture.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/texture2d.hpp>

namespace mbgl {
namespace vulkan {

class OffscreenTextureResource final : public RenderableResource {
public:
    OffscreenTextureResource(RendererBackend& backend_, const Size size_, const gfx::TextureChannelDataType type_)
        : RenderableResource(backend_),
          size(size_),
          type(type_) {
        assert(!size.isEmpty());

        extent.width = size.width;
        extent.height = size.height;

        colorTexture = backend.getContext().createTexture2D();
        auto& texture = static_cast<Texture2D&>(*colorTexture);

        texture.setSize(size);
        texture.setFormat(gfx::TexturePixelType::RGBA, type);
        texture.setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        texture.setUsage(Texture2DUsage::Attachment);

        backend.getContext().renderingStats().numFrameBuffers++;
    }

    ~OffscreenTextureResource() noexcept override {
        framebuffer.reset();
        renderPass.reset();
        colorTexture.reset();

        backend.getContext().renderingStats().numFrameBuffers--;
    }

    void bind() override {
        colorTexture->create();
        createRenderPass();
    }

    const vk::UniqueFramebuffer& getFramebuffer() const override { return framebuffer; };

    PremultipliedImage readStillImage() {
        if (!colorTexture) {
            return {};
        }

        const auto& image = static_cast<Texture2D&>(*colorTexture).readImage();
        return image ? image->clone() : PremultipliedImage();
    }

    gfx::Texture2DPtr& getTexture() {
        assert(colorTexture);
        return colorTexture;
    }

    void createRenderPass() {
        if (renderPass) return;

        assert(colorTexture);
        auto& texture = static_cast<Texture2D&>(*colorTexture);

        const auto colorAttachment = vk::AttachmentDescription(vk::AttachmentDescriptionFlags())
                                         .setFormat(texture.getVulkanFormat())
                                         .setSamples(vk::SampleCountFlagBits::e1)
                                         .setLoadOp(vk::AttachmentLoadOp::eClear)
                                         .setStoreOp(vk::AttachmentStoreOp::eStore)
                                         .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                                         .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                                         .setInitialLayout(vk::ImageLayout::eUndefined)
                                         .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        const vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

        const auto subpass = vk::SubpassDescription(vk::SubpassDescriptionFlags())
                                 .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                                 .setColorAttachments(colorAttachmentRef);

        const auto subpassSrcStageMask = vk::PipelineStageFlags() | vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                         vk::PipelineStageFlagBits::eLateFragmentTests;

        const auto subpassDstStageMask = vk::PipelineStageFlags() | vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                         vk::PipelineStageFlagBits::eEarlyFragmentTests;

        const auto subpassSrcAccessMask = vk::AccessFlags() | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        const auto subpassDstAccessMask = vk::AccessFlags() | vk::AccessFlagBits::eColorAttachmentWrite |
                                          vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        const auto subpassDependency = vk::SubpassDependency()
                                           .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                                           .setDstSubpass(0)
                                           .setSrcStageMask(subpassSrcStageMask)
                                           .setDstStageMask(subpassDstStageMask)
                                           .setSrcAccessMask(subpassSrcAccessMask)
                                           .setDstAccessMask(subpassDstAccessMask);

        const auto renderPassCreateInfo = vk::RenderPassCreateInfo()
                                              .setAttachments(colorAttachment)
                                              .setSubpasses(subpass)
                                              .setDependencies(subpassDependency);

        renderPass = backend.getDevice()->createRenderPassUnique(
            renderPassCreateInfo, nullptr, backend.getDispatcher());

        const auto framebufferCreateInfo = vk::FramebufferCreateInfo()
                                               .setRenderPass(renderPass.get())
                                               .setAttachments(texture.getVulkanImageView().get())
                                               .setAttachmentCount(1)
                                               .setWidth(extent.width)
                                               .setHeight(extent.height)
                                               .setLayers(1);

        framebuffer = backend.getDevice()->createFramebufferUnique(
            framebufferCreateInfo, nullptr, backend.getDispatcher());
    }

private:
    const Size size;
    const gfx::TextureChannelDataType type;

    gfx::Texture2DPtr colorTexture;
    vk::UniqueFramebuffer framebuffer;
};

OffscreenTexture::OffscreenTexture(Context& context,
                                   const Size size_,
                                   const gfx::TextureChannelDataType type,
                                   [[maybe_unused]] bool depth,
                                   [[maybe_unused]] bool stencil)
    : gfx::OffscreenTexture(size, std::make_unique<OffscreenTextureResource>(context.getBackend(), size_, type)) {
    assert(!depth);
    assert(!stencil);
}

bool OffscreenTexture::isRenderable() {
    return true;
}

PremultipliedImage OffscreenTexture::readStillImage() {
    return getResource<OffscreenTextureResource>().readStillImage(); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
}

const gfx::Texture2DPtr& OffscreenTexture::getTexture() {
    return getResource<OffscreenTextureResource>().getTexture();
}

} // namespace vulkan
} // namespace mbgl
