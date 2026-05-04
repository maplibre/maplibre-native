#pragma once

#include <mbgl/gfx/dynamic_texture.hpp>
#include <mbgl/vulkan/texture2d.hpp>

#define DYNAMIC_TEXTURE_VULKAN_MULTITHREADED_UPLOAD 0

namespace mbgl {
namespace vulkan {

class Context;
struct BufferAllocation;
using UniqueBufferAllocation = std::unique_ptr<BufferAllocation>;

class DynamicTexture : public gfx::DynamicTexture {
public:
    DynamicTexture(Context& context, Size size, gfx::TexturePixelType pixelType);
    ~DynamicTexture() override;

    void uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) override;
    void uploadDeferredImages() override;
    bool removeTexture(const gfx::TextureHandle& texHandle) override;

#if DYNAMIC_TEXTURE_VULKAN_MULTITHREADED_UPLOAD
    using TexturesToBlit =
        std::unordered_map<gfx::TextureHandle, std::shared_ptr<vulkan::Texture2D>, gfx::TextureHandle::Hasher>;
#else
    using TextureBuffersToUpload =
        std::unordered_map<gfx::TextureHandle, UniqueBufferAllocation, gfx::TextureHandle::Hasher>;
#endif

private:
    vulkan::Context& context;

#if DYNAMIC_TEXTURE_VULKAN_MULTITHREADED_UPLOAD
    TexturesToBlit texturesToBlit;
    vk::UniqueCommandPool commandPool;
#else
    TextureBuffersToUpload textureBuffersToUpload;
#endif
};

} // namespace vulkan
} // namespace mbgl
