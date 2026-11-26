#pragma once

#include <mln/gfx/backend.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/util/image.hpp>

#include <android/native_window.h>

namespace mln {
namespace android {

class AndroidRendererBackend {
public:
    AndroidRendererBackend() = default;
    AndroidRendererBackend(const AndroidRendererBackend&) = delete;
    AndroidRendererBackend& operator=(const AndroidRendererBackend&) = delete;
    virtual ~AndroidRendererBackend() = default;

    static std::unique_ptr<AndroidRendererBackend> Create(ANativeWindow* window) {
        return mln::gfx::Backend::Create<AndroidRendererBackend, ANativeWindow*>(window);
    }
    virtual mln::gfx::RendererBackend& getImpl() = 0;

    virtual void updateViewPort();

    // Ensures the current context is not cleaned up when destroyed
    virtual void markContextLost();

    virtual bool createSurface(ANativeWindow*) { return false; }
    virtual void destroySurface() {}

    virtual void resizeFramebuffer(int width, int height);
    virtual void enableFramebufferRead(bool value) {}
    virtual PremultipliedImage readFramebuffer();

    gfx::Renderable::SwapBehaviour getSwapBehavior() const { return swapBehaviour; }
    virtual void setSwapBehavior(gfx::Renderable::SwapBehaviour swapBehaviour);

protected:
    gfx::Renderable::SwapBehaviour swapBehaviour = gfx::Renderable::SwapBehaviour::NoFlush;
};

} // namespace android
} // namespace mln
