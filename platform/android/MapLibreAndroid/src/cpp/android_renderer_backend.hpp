#pragma once

#include <mbgl/gfx/backend.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/util/image.hpp>

#include <android/native_window.h>

namespace mbgl {
namespace android {

class AndroidRendererBackend {
public:
    AndroidRendererBackend() = default;
    AndroidRendererBackend(const AndroidRendererBackend&) = delete;
    AndroidRendererBackend& operator=(const AndroidRendererBackend&) = delete;
    virtual ~AndroidRendererBackend() = default;

    static std::unique_ptr<AndroidRendererBackend> Create(ANativeWindow* window) {
        return mbgl::gfx::Backend::Create<AndroidRendererBackend, ANativeWindow*>(window);
    }
    virtual mbgl::gfx::RendererBackend& getImpl() = 0;

    virtual void updateViewPort();

    // Ensures the current context is not cleaned up when destroyed
    virtual void markContextLost();

    virtual void resizeFramebuffer(int width, int height);
    virtual PremultipliedImage readFramebuffer();

    gfx::Renderable::SwapBehaviour getSwapBehavior() const { return swapBehaviour; }
    virtual void setSwapBehavior(gfx::Renderable::SwapBehaviour swapBehaviour);

protected:
    gfx::Renderable::SwapBehaviour swapBehaviour = gfx::Renderable::SwapBehaviour::NoFlush;
};

} // namespace android
} // namespace mbgl
