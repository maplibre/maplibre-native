#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/shaders/webgpu/shader_group.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

class RendererBackend::Impl : public gfx::Renderable {
public:
    Impl() : gfx::Renderable({800, 600}, nullptr) {}  // Default size
    ~Impl() override = default;

    void* wgpuInstance = nullptr;
    void* wgpuDevice = nullptr;
    void* wgpuQueue = nullptr;
    void* wgpuSurface = nullptr;
};

RendererBackend::RendererBackend(gfx::ContextMode mode)
    : gfx::RendererBackend(mode),
      impl(std::make_unique<Impl>()) {
}

RendererBackend::~RendererBackend() {
    // Explicitly defined destructor to ensure vtable is generated
}

gfx::Renderable& RendererBackend::getDefaultRenderable() {
    return *impl;
}

void RendererBackend::initShaders(gfx::ShaderRegistry& registry, const ProgramParameters& parameters) {
    // Initialize WebGPU shaders by registering shader groups
    // The actual shader compilation happens lazily when they're first used

    // Get the context to initialize shader groups
    auto contextPtr = createContext();
    auto& ctx = static_cast<webgpu::Context&>(*contextPtr);

    // Create ONE shader group that contains all shaders
    auto webgpuShaderGroup = std::make_shared<webgpu::ShaderGroup>();
    webgpuShaderGroup->initialize(ctx);

    // Register the same shader group under multiple names for different layer types
    // This allows layers to find their shaders either by specific name or through the group
    const std::vector<std::string> shaderGroupNames = {
        "FillShader",
        "LineShader",
        "CircleShader",
        "BackgroundShader",
        "RasterShader",
        "HillshadeShader",
        "FillExtrusionShader",
        "HeatmapShader",
        "SymbolShader"
    };

    for (const auto& groupName : shaderGroupNames) {
        // Clone the shared pointer for each registration
        auto groupClone = webgpuShaderGroup;
        registry.registerShaderGroup(std::move(groupClone), groupName);
    }

    (void)parameters;
}

void RendererBackend::setSurface(void* nativeWindow) {
    // Platform-specific surface creation
    // This will be used to create the WebGPU surface from a native window handle
    impl->wgpuSurface = nativeWindow;

}

void RendererBackend::setInstance(void* instance) {
    impl->wgpuInstance = instance;
}

void RendererBackend::setDevice(void* device) {
    impl->wgpuDevice = device;
}

void RendererBackend::setQueue(void* queue) {
    impl->wgpuQueue = queue;
}

void* RendererBackend::getInstance() const {
    return impl->wgpuInstance;
}

void* RendererBackend::getDevice() const {
    return impl->wgpuDevice;
}

void* RendererBackend::getQueue() const {
    return impl->wgpuQueue;
}

void* RendererBackend::getSurface() const {
    return impl->wgpuSurface;
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

void* RendererBackend::getCurrentTextureView() {
    // Default implementation - platform backends should override
    return nullptr;
}

void* RendererBackend::getDepthStencilView() {
    // Default implementation - platform backends should override
    return nullptr;
}

mbgl::Size RendererBackend::getFramebufferSize() const {
    // Default implementation - platform backends should override
    return {800, 600};
}

} // namespace webgpu
} // namespace mbgl
