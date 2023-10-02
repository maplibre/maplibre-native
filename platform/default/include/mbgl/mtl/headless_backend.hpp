#pragma once

#include <mbgl/gfx/headless_backend.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <memory>
#include <functional>

namespace mbgl {
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
    RendererBackend* getRendererBackend() override;
    SwapBehaviour getSwapBehaviour();

private:
    void activate() override;
    void deactivate() override;

private:
    bool active = false;
    SwapBehaviour swapBehaviour = SwapBehaviour::NoFlush;
};

} // namespace mtl
} // namespace mbgl
