#pragma once

#include <mbgl/gfx/dynamic_texture.hpp>

#define DYNAMIC_TEXTURE_VULKAN_MULTITHREADED_UPLOAD     1

namespace mbgl {
namespace vulkan {

class Context;
struct BufferAllocation;
using UniqueBufferAllocation = std::unique_ptr<BufferAllocation>;

class DynamicTexture : public gfx::DynamicTexture {
public:
    DynamicTexture(Context& context, Size size, gfx::TexturePixelType pixelType);

    void uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) override;
    void uploadDeferredImages() override;
    bool removeTexture(const gfx::TextureHandle& texHandle) override;

#if DYNAMIC_TEXTURE_VULKAN_MULTITHREADED_UPLOAD
    using TexturesToBlit = std::unordered_map<gfx::TextureHandle, gfx::Texture2DPtr, gfx::TextureHandle::Hasher>;
#else
    using TextureBuffersToUpload = std::unordered_map<gfx::TextureHandle, UniqueBufferAllocation, gfx::TextureHandle::Hasher>;
#endif

private:
    Context& context;

#if DYNAMIC_TEXTURE_VULKAN_MULTITHREADED_UPLOAD
    TexturesToBlit texturesToBlit;
#else
    TextureBuffersToUpload textureBuffersToUpload;
#endif
};

} // namespace vulkan
} // namespace mbgl
