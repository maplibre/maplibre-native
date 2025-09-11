#pragma once

#include <mbgl/gfx/headless_backend.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <memory>

namespace mbgl {
namespace webgpu {

class HeadlessBackend final : public webgpu::RendererBackend, public gfx::HeadlessBackend {
public:
    HeadlessBackend(Size = {256, 256},
                    SwapBehaviour = SwapBehaviour::NoFlush,
                    gfx::ContextMode = gfx::ContextMode::Unique);
    ~HeadlessBackend() override;
    
    gfx::Renderable& getDefaultRenderable() override;
    PremultipliedImage readStillImage() override;
    RendererBackend* getRendererBackend() override;

private:
    void activate() override;
    void deactivate() override;

private:
    bool active = false;
    SwapBehaviour swapBehaviour = SwapBehaviour::NoFlush;
};

} // namespace webgpu
} // namespace mbgl