#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/size.hpp>

// Include shader group and individual shader headers
#include <mbgl/shaders/webgpu/shader_group.hpp>
#include <mbgl/shaders/webgpu/fill.hpp>

namespace mbgl {
namespace webgpu {

// Forward declare and define the Impl class
class RendererBackend::Impl {
public:
    void* instance = nullptr;
    void* device = nullptr;
    void* queue = nullptr;
    void* surface = nullptr;
};

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_),
      impl(std::make_unique<Impl>()) {
}

RendererBackend::~RendererBackend() = default;

void RendererBackend::activate() {
    // Activation logic if needed
}

void RendererBackend::deactivate() {
    // Deactivation logic if needed
}

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    auto ctx = std::make_unique<Context>(*this);
    return ctx;
}

// getDefaultRenderable() is pure virtual and must be implemented by platform-specific backends
// The platform backend (e.g., GLFWWebGPUBackend) typically inherits from gfx::Renderable itself
// and returns *this from getDefaultRenderable()

namespace {

template <shaders::BuiltIn... ShaderID>
void registerTypes(gfx::ShaderRegistry& registry, const ProgramParameters& programParameters) {
    using namespace std::string_literals;

    // Register each shader type using fold expression
    ([&]() {
        using ShaderClass = shaders::ShaderSource<ShaderID, gfx::Backend::Type::WebGPU>;
        auto group = std::make_shared<webgpu::ShaderGroup<ShaderID>>(programParameters);
        if (!registry.registerShaderGroup(std::move(group), ShaderClass::name)) {
            assert(!"duplicate shader group");
            throw std::runtime_error("Failed to register "s + ShaderClass::name + " with shader registry!");
        }
    }(), ...);
}

} // namespace

void RendererBackend::initShaders(gfx::ShaderRegistry& registry, const ProgramParameters& parameters) {
    // Register just the FillShader for now - more will be added as their headers are created
    registerTypes<shaders::BuiltIn::FillShader>(registry, parameters);

    // TODO: Add more shader types as their WebGPU implementations are created:
    // registerTypes<
    //     shaders::BuiltIn::FillShader,
    //     shaders::BuiltIn::LineShader,
    //     shaders::BuiltIn::CircleShader,
    //     shaders::BuiltIn::BackgroundShader,
    //     ...
    // >(registry, parameters);
}

void RendererBackend::setSurface(void* nativeWindow) {
    // Platform-specific surface creation will be handled by subclasses
    // For now, just store the native window handle
    // The actual surface creation depends on the platform (X11, Wayland, etc.)
}

void RendererBackend::setInstance(void* instance) {
    impl->instance = instance;
}

void RendererBackend::setDevice(void* device) {
    impl->device = device;
}

void RendererBackend::setQueue(void* queue) {
    impl->queue = queue;
}

void* RendererBackend::getInstance() const {
    return impl->instance;
}

void* RendererBackend::getDevice() const {
    return impl->device;
}

void* RendererBackend::getQueue() const {
    return impl->queue;
}

void* RendererBackend::getSurface() const {
    return impl->surface;
}

void* RendererBackend::getCurrentTextureView() {
    // Default implementation - platform backends can override if needed
    return nullptr;
}

void* RendererBackend::getDepthStencilView() {
    // Default implementation - platform backends can override if needed
    return nullptr;
}

mbgl::Size RendererBackend::getFramebufferSize() const {
    // Default implementation - platform backends should override
    return {0, 0};
}

} // namespace webgpu
} // namespace mbgl