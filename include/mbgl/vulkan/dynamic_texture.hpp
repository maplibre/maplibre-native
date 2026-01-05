#pragma once

#include <mbgl/gfx/dynamic_texture.hpp>

namespace mbgl {
namespace vulkan {

class Context;

class DynamicTexture : public gfx::DynamicTexture {
public:
    DynamicTexture(Context& context, Size size, gfx::TexturePixelType pixelType);

    void uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) override;

private:
    Context& context;
};

} // namespace vulkan
} // namespace mbgl
