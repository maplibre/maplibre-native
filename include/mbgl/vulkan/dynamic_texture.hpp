#pragma once

#include <mbgl/gfx/dynamic_texture.hpp>

namespace mbgl {
namespace vulkan {

class Context;

class DynamicTexture : public gfx::DynamicTexture {
public:
    DynamicTexture(Context& context, Size size, gfx::TexturePixelType pixelType);

    void uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) override;
    void uploadDeferredImages() override;
    bool removeTexture(const gfx::TextureHandle& texHandle) override;

    using TexturesToBlit = std::unordered_map<gfx::TextureHandle, gfx::Texture2DPtr, gfx::TextureHandle::Hasher>;

private:
    Context& context;
    TexturesToBlit texturesToBlit;
};

} // namespace vulkan
} // namespace mbgl
