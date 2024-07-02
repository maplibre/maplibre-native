#pragma once

#include <mbgl/gfx/headless_backend.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <memory>
#include <functional>

namespace mbgl {
namespace vulkan {

class HeadlessBackend final : public vulkan::RendererBackend, public gfx::HeadlessBackend {
public:
    HeadlessBackend(Size = {256, 256},
                    SwapBehaviour = SwapBehaviour::NoFlush,
                    gfx::ContextMode = gfx::ContextMode::Unique);
    ~HeadlessBackend() override;
    gfx::Renderable& getDefaultRenderable() override;
    PremultipliedImage readStillImage() override;
    RendererBackend* getRendererBackend() override;

    void swap();

    class Impl {
    public:
        virtual ~Impl() = default;
        virtual bool glNeedsActiveContextOnDestruction() const { return false; }
        virtual void activateContext() {}
        virtual void deactivateContext() {}
        virtual void getContext() {}
    };

private:
    void activate() override;
    void deactivate() override;

    void createImpl();

private:
    std::unique_ptr<Impl> impl;
    bool active = false;
    SwapBehaviour swapBehaviour = SwapBehaviour::NoFlush;
};

} // namespace vulkan
} // namespace mbgl
