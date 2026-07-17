#include <mbgl/vulkan/offscreen_texture.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/util/logging.hpp>

#include <algorithm>
#include <array>
#include <vector>

namespace mbgl {
namespace vulkan {

class OffscreenTextureResource final : public RenderableResource {
public:
    OffscreenTextureResource(RendererBackend& backend_,
                             const Size size_,
                             const gfx::TextureChannelDataType type_,
                             bool stencil_)
        : RenderableResource(backend_),
          size(size_),
          type(type_),
          wantsStencil(stencil_) {
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
        depthAllocation.reset();
        colorTexture.reset();

        backend.getContext().renderingStats().numFrameBuffers--;
    }

    void bind() override {
        colorTexture->create();
        createDepthImage();
        createRenderPass();
    }

    const vk::UniqueFramebuffer& getFramebuffer() const override { return framebuffer; };

    // True once createDepthImage() has picked a combined depth/stencil format; false for a
    // depth-only target or if stencil support was requested but no supported format was found
    // (createDepthImage() logs an error and hasStencilAttachment() falls back to depth-only so
    // the target still renders, just without tile-clipping).
    bool hasStencilAttachment() const override { return stencilSupported; }

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

    // Create the depth (and, for drape render targets, stencil) attachment for the offscreen
    // target. Terrain drape targets need a stencil to clip overlapping draped tiles against
    // each other (TexturePool::createRenderTarget), matching gl-js's render-to-texture
    // framebuffer; other offscreen textures (e.g. the depth-occlusion pass) stay depth-only.
    // eD32Sfloat is a Vulkan-mandated depth-only format; a combined depth/stencil format is
    // not universally guaranteed, so it is queried the same way
    // SurfaceRenderableResource::initDepthStencil() does for the main framebuffer.
    void createDepthImage() {
        if (depthAllocation) return;

        if (wantsStencil) {
            const auto& physicalDevice = backend.getPhysicalDevice();
            const auto& dispatcher = backend.getDispatcher();
            const std::vector<vk::Format> formats = {
                vk::Format::eD24UnormS8Uint,
                vk::Format::eD32SfloatS8Uint,
                vk::Format::eD16UnormS8Uint,
            };
            const auto& formatIt = std::find_if(formats.begin(), formats.end(), [&](const auto& format) {
                const auto& formatProps = physicalDevice.getFormatProperties(format, dispatcher);
                return formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment;
            });
            if (formatIt != formats.end()) {
                depthFormat = *formatIt;
                stencilSupported = true;
            } else {
                mbgl::Log::Error(mbgl::Event::Render, "Depth/Stencil format not available for offscreen texture");
            }
        }

        const auto imageCreateInfo = vk::ImageCreateInfo()
                                         .setImageType(vk::ImageType::e2D)
                                         .setFormat(depthFormat)
                                         .setExtent({extent.width, extent.height, 1})
                                         .setMipLevels(1)
                                         .setArrayLayers(1)
                                         .setSamples(vk::SampleCountFlagBits::e1)
                                         .setTiling(vk::ImageTiling::eOptimal)
                                         .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
                                         .setSharingMode(vk::SharingMode::eExclusive)
                                         .setInitialLayout(vk::ImageLayout::eUndefined);

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        depthAllocation = std::make_unique<ImageAllocation>(backend.getAllocator());
        if (!depthAllocation->create(allocCreateInfo, imageCreateInfo)) {
            mbgl::Log::Error(mbgl::Event::Render, "Vulkan offscreen depth allocation failed");
            depthAllocation.reset();
            return;
        }

        const auto depthAspect = stencilSupported ? (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil)
                                                  : vk::ImageAspectFlags(vk::ImageAspectFlagBits::eDepth);
        const auto imageViewCreateInfo = vk::ImageViewCreateInfo()
                                             .setImage(depthAllocation->image)
                                             .setViewType(vk::ImageViewType::e2D)
                                             .setFormat(depthFormat)
                                             .setSubresourceRange(vk::ImageSubresourceRange(depthAspect, 0, 1, 0, 1));

        depthAllocation->imageView = backend.getDevice()->createImageViewUnique(
            imageViewCreateInfo, nullptr, backend.getDispatcher());
    }

    void createRenderPass() {
        if (renderPass) return;

        assert(colorTexture);
        auto& texture = static_cast<Texture2D&>(*colorTexture);

        const std::array<vk::AttachmentDescription, 2> attachments = {
            vk::AttachmentDescription(vk::AttachmentDescriptionFlags())
                .setFormat(texture.getVulkanFormat())
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),

            vk::AttachmentDescription(vk::AttachmentDescriptionFlags())
                .setFormat(depthFormat)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setStencilLoadOp(stencilSupported ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)};

        const vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
        const vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        const auto subpass = vk::SubpassDescription(vk::SubpassDescriptionFlags())
                                 .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                                 .setColorAttachments(colorAttachmentRef)
                                 .setPDepthStencilAttachment(&depthAttachmentRef);

        const std::array<vk::SubpassDependency, 2> dependencies = {
            vk::SubpassDependency()
                .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDependencyFlags(vk::DependencyFlagBits::eByRegion),

            vk::SubpassDependency()
                .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests |
                                 vk::PipelineStageFlagBits::eLateFragmentTests)
                .setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests |
                                 vk::PipelineStageFlagBits::eLateFragmentTests)
                .setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
                .setDependencyFlags(vk::DependencyFlagBits::eByRegion)};

        const auto renderPassCreateInfo =
            vk::RenderPassCreateInfo().setAttachments(attachments).setSubpasses(subpass).setDependencies(dependencies);

        renderPass = backend.getDevice()->createRenderPassUnique(
            renderPassCreateInfo, nullptr, backend.getDispatcher());

        const std::array<vk::ImageView, 2> framebufferAttachments = {texture.getVulkanImageView().get(),
                                                                     depthAllocation->imageView.get()};

        const auto framebufferCreateInfo = vk::FramebufferCreateInfo()
                                               .setRenderPass(renderPass.get())
                                               .setAttachments(framebufferAttachments)
                                               .setWidth(extent.width)
                                               .setHeight(extent.height)
                                               .setLayers(1);

        framebuffer = backend.getDevice()->createFramebufferUnique(
            framebufferCreateInfo, nullptr, backend.getDispatcher());
    }

private:
    const Size size;
    const gfx::TextureChannelDataType type;
    const bool wantsStencil;
    bool stencilSupported = false;

    gfx::Texture2DPtr colorTexture;
    UniqueImageAllocation depthAllocation;
    vk::Format depthFormat{vk::Format::eD32Sfloat};
    vk::UniqueFramebuffer framebuffer;
};

OffscreenTexture::OffscreenTexture(Context& context,
                                   const Size size_,
                                   const gfx::TextureChannelDataType type,
                                   [[maybe_unused]] bool depth,
                                   bool stencil)
    : gfx::OffscreenTexture(
          size, std::make_unique<OffscreenTextureResource>(context.getBackend(), size_, type, stencil)) {}

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
