#pragma once

#include <mbgl/gfx/backend.hpp>
#include <mbgl/gfx/custom_puck.hpp>
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

    static std::unique_ptr<AndroidRendererBackend> Create(ANativeWindow* window, bool multiThreadedGpuResourceUpload) {
        return mbgl::gfx::Backend::Create<AndroidRendererBackend, ANativeWindow*, bool>(window,
                                                                                        multiThreadedGpuResourceUpload);
    }
    virtual mbgl::gfx::RendererBackend& getImpl() = 0;

    virtual void updateViewPort();

    // Ensures the current context is not cleaned up when destroyed
    virtual void markContextLost();

    virtual void resizeFramebuffer(int width, int height);
    virtual PremultipliedImage readFramebuffer();

    gfx::Renderable::SwapBehaviour getSwapBehavior() const { return swapBehaviour; }
    virtual void setSwapBehavior(gfx::Renderable::SwapBehaviour swapBehaviour);

    void setPuckBitmap(const PremultipliedImage& image);

    void setCustomPuckState(const gfx::CustomPuckState& state) noexcept { customPuckState = state; }

    const gfx::CustomPuckState& getCustomPuckState() const noexcept { return customPuckState; }

protected:
    gfx::Renderable::SwapBehaviour swapBehaviour = gfx::Renderable::SwapBehaviour::NoFlush;
    gfx::CustomPuckState customPuckState;
};

} // namespace android
} // namespace mbgl
