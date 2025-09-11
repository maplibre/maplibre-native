#include <mbgl/webgpu/headless_backend.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

HeadlessBackend::HeadlessBackend(Size size, SwapBehaviour swapBehaviour_, gfx::ContextMode contextMode)
    : webgpu::RendererBackend(contextMode),
      gfx::HeadlessBackend(size),
      swapBehaviour(swapBehaviour_) {
    (void)swapBehaviour; // TODO: Use this when implementing swap buffer behavior
    mbgl::Log::Info(mbgl::Event::General, "Initializing WebGPU headless backend");
    // TODO: Initialize WebGPU context for headless rendering
}

HeadlessBackend::~HeadlessBackend() = default;

gfx::Renderable& HeadlessBackend::getDefaultRenderable() {
    return *this;
}

PremultipliedImage HeadlessBackend::readStillImage() {
    // TODO: Implement reading image from WebGPU framebuffer
    return PremultipliedImage(getSize());
}

RendererBackend* HeadlessBackend::getRendererBackend() {
    return this;
}

void HeadlessBackend::activate() {
    active = true;
}

void HeadlessBackend::deactivate() {
    active = false;
}

} // namespace webgpu

namespace gfx {

template <>
std::unique_ptr<gfx::HeadlessBackend> Backend::Create<Backend::Type::WebGPU>(
    Size size,
    gfx::Renderable::SwapBehaviour swapBehaviour,
    gfx::ContextMode contextMode) {
    return std::make_unique<webgpu::HeadlessBackend>(size, swapBehaviour, contextMode);
}

} // namespace gfx
} // namespace mbgl