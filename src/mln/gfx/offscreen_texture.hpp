#pragma once

#include <mln/gfx/renderable.hpp>
#include <mln/util/image.hpp>

#include <cstdint>

namespace mln {
namespace gfx {

class Texture;
class Texture2D;
using Texture2DPtr = std::shared_ptr<Texture2D>;

class OffscreenTexture : public gfx::Renderable {
protected:
    OffscreenTexture(const Size size_, std::unique_ptr<RenderableResource> resource_)
        : gfx::Renderable(size_, std::move(resource_)) {}

public:
    ~OffscreenTexture() override = default;

    virtual bool isRenderable() = 0;

    virtual PremultipliedImage readStillImage() = 0;

    virtual const gfx::Texture2DPtr& getTexture() = 0;

    /// Give the color texture a mip chain, rebuilt each time the texture is rendered to (presented),
    /// and sample it trilinearly with up to `maxAnisotropy` taps where the backend supports it. For a
    /// texture that is drawn onto a surface seen at an angle or from afar, such as a terrain drape:
    /// from the base level alone, thin content steps over whole texels and breaks into dashes.
    /// Costs a third more texture memory. Backends without support keep the texture as it is.
    virtual void setMipmapped(bool /*mipmapped*/, uint8_t /*maxAnisotropy*/) {}
};

} // namespace gfx
} // namespace mln
