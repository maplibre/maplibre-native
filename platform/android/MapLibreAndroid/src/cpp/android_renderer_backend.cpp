#include "android_renderer_backend.hpp"
#include <mbgl/util/logging.hpp>
#include <cassert>

namespace mbgl {
namespace android {

void AndroidRendererBackend::updateViewPort() {}

void AndroidRendererBackend::resizeFramebuffer(int width, int height) {}

PremultipliedImage AndroidRendererBackend::readFramebuffer() {
    return PremultipliedImage();
}

void AndroidRendererBackend::markContextLost() {}

void AndroidRendererBackend::setSwapBehavior(gfx::Renderable::SwapBehaviour swapBehaviour_) {
    swapBehaviour = swapBehaviour_;
}

void AndroidRendererBackend::setPuckBitmap(const PremultipliedImage& image) {
    if (!getImpl().customPuck) {
        Log::Debug(Event::Android, "Custom puck not enabled");
        return;
    }
    getImpl().customPuck->setPuckBitmap(image);
}

} // namespace android
} // namespace mbgl
