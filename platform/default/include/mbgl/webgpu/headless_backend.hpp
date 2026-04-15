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

    // Override WebGPU-specific methods
    void* getCurrentTextureView() override;
    void* getDepthStencilView() override;
    mbgl::Size getFramebufferSize() const override;

private:
    void activate() override;
    void deactivate() override;
    void createOffscreenTextures();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
    bool active = false;
};

} // namespace webgpu
} // namespace mbgl
