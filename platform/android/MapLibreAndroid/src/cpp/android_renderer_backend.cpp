#include "android_renderer_backend.hpp"
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

} // namespace android
} // namespace mbgl
