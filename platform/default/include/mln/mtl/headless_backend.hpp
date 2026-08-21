#pragma once

#include <mln/gfx/headless_backend.hpp>
#include <mln/mtl/mtl_fwd.hpp>
#include <mln/mtl/renderer_backend.hpp>
#include <memory>
#include <functional>

namespace mln {
namespace mtl {

class HeadlessBackend final : public mtl::RendererBackend, public gfx::HeadlessBackend {
public:
    HeadlessBackend(Size = {256, 256},
                    SwapBehaviour = SwapBehaviour::NoFlush,
                    gfx::ContextMode = gfx::ContextMode::Unique);
    ~HeadlessBackend() override;
    void updateAssumedState() override;
    gfx::Renderable& getDefaultRenderable() override;
    PremultipliedImage readStillImage() override;
    MTL::Texture* getMetalTexture();
    RendererBackend* getRendererBackend() override;
    SwapBehaviour getSwapBehaviour();

private:
    void activate() override;
    void deactivate() override;
    void ensureResource();

private:
    bool active = false;
    SwapBehaviour swapBehaviour = SwapBehaviour::NoFlush;
};

} // namespace mtl
} // namespace mln
