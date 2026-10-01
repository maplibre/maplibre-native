#pragma once

#include <mln/gfx/headless_backend.hpp>
#include <mln/webgpu/renderer_backend.hpp>
#include "mln/gfx/offscreen_texture.hpp"
#include <memory>

namespace mln {
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
    /*!
     * \brief takeTexture
     * Take ownership of the current texture
     * \return
     */
    gfx::Texture2DPtr takeTexture();

    // Override WebGPU-specific methods
    void* getCurrentTextureView() override;
    void* getDepthStencilView() override;
    mln::Size getFramebufferSize() const override;

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
} // namespace mln
