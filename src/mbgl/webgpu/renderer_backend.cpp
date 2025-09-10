#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

class RendererBackend::Impl : public gfx::Renderable {
public:
    Impl() = default;
    ~Impl() override = default;
};

RendererBackend::RendererBackend(gfx::ContextMode contextMode)
    : gfx::RendererBackend(contextMode),
      impl(std::make_unique<Impl>()) {
    Log::Info(Event::General, "Initializing WebGPU renderer backend");
}

RendererBackend::~RendererBackend() = default;

gfx::Renderable& RendererBackend::getDefaultRenderable() {
    return *impl;
}

void RendererBackend::initShaders(gfx::ShaderRegistry& registry, const ProgramParameters& parameters) {
    // TODO: Initialize WebGPU shaders
    // This will compile and register all the shaders needed for rendering
    Log::Info(Event::General, "Initializing WebGPU shaders");
}

void RendererBackend::setSurface(void* nativeWindow) {
    // Platform-specific surface creation
    // This will be used to create the WebGPU surface from a native window handle
    Log::Info(Event::General, "Setting WebGPU surface");
}

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    return std::make_unique<Context>(*this);
}

void RendererBackend::activate() {
    // Make the WebGPU context current
}

void RendererBackend::deactivate() {
    // Release the current WebGPU context
}

} // namespace webgpu
} // namespace mbgl