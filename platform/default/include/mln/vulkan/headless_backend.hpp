#pragma once

#include <mln/gfx/headless_backend.hpp>
#include <mln/vulkan/renderer_backend.hpp>
#include <mln/vulkan/shared_device.hpp>
#include <memory>
#include <functional>

namespace mln {
namespace vulkan {

class Texture2D;

class HeadlessBackend final : public vulkan::RendererBackend, public gfx::HeadlessBackend {
public:
    /// Pass a `SharedDevice` to use its instance, device and allocator instead of creating new ones.
    HeadlessBackend(Size = {256, 256},
                    SwapBehaviour = SwapBehaviour::NoFlush,
                    gfx::ContextMode = gfx::ContextMode::Unique,
                    std::shared_ptr<SharedDevice> = nullptr);
    ~HeadlessBackend() override;
    gfx::Renderable& getDefaultRenderable() override;
    PremultipliedImage readStillImage() override;
    RendererBackend* getRendererBackend() override;

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
};

} // namespace vulkan
} // namespace mln
