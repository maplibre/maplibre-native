#pragma once

#include <mbgl/gfx/dynamic_texture.hpp>

namespace mbgl {
namespace gl {

class Context;

class DynamicTexture : public gfx::DynamicTexture {
public:
    DynamicTexture(Context& context, Size size, gfx::TexturePixelType pixelType);

    void uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) override;
    void uploadDeferredImages() override;
    bool removeTexture(const gfx::TextureHandle& texHandle) override;

    using ImagesToUpload =
        std::unordered_map<gfx::TextureHandle, std::unique_ptr<uint8_t[]>, gfx::TextureHandle::Hasher>;

private:
    Context& context;
    bool deferredCreation = false;
    ImagesToUpload imagesToUpload;
};

} // namespace gl
} // namespace mbgl
