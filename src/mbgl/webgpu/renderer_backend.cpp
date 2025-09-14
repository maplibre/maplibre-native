#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/logging.hpp>

// Include shader group and individual shader headers
#include <mbgl/shaders/webgpu/shader_group.hpp>
#include <mbgl/shaders/webgpu/fill.hpp>

namespace mbgl {
namespace webgpu {

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_) {
}

RendererBackend::~RendererBackend() = default;

void RendererBackend::bind() {
    gfx::RendererBackend::bind();
}

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    auto context = std::make_unique<Context>(*this);
    return context;
}

gfx::BackendScope RendererBackend::getDefaultRenderable() {
    return gfx::BackendScope{};
}

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

void RendererBackend::updateSurface(wgpu::Surface surface) {
    this->surface = surface;
}

wgpu::Device RendererBackend::getDevice() const {
    if (device) {
        return device;
    }

    // Initialize WebGPU if not already done
    const_cast<RendererBackend*>(this)->initializeDevice();
    return device;
}

wgpu::Surface RendererBackend::getSurface() const {
    return surface;
}

void RendererBackend::initializeDevice() {
    if (device) {
        return;
    }

    // Create instance
    wgpu::InstanceDescriptor desc{};
    instance = wgpu::CreateInstance(&desc);

    // Request adapter
    adapter = nullptr;
    instance.RequestAdapter(
        nullptr,
        [](WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userdata) {
            if (status == WGPURequestAdapterStatus_Success) {
                *static_cast<wgpu::Adapter*>(userdata) = wgpu::Adapter(adapter);
            } else {
                Log::Error(Event::Render, "Failed to get WebGPU adapter: %s", message);
            }
        },
        &adapter
    );

    // Wait for adapter (in production, this should be async)
    while (!adapter) {
        // Process callbacks - implementation depends on the backend
    }

    // Request device
    device = nullptr;
    adapter.RequestDevice(
        nullptr,
        [](WGPURequestDeviceStatus status, WGPUDevice device, const char* message, void* userdata) {
            if (status == WGPURequestDeviceStatus_Success) {
                *static_cast<wgpu::Device*>(userdata) = wgpu::Device(device);
            } else {
                Log::Error(Event::Render, "Failed to get WebGPU device: %s", message);
            }
        },
        &device
    );

    // Wait for device (in production, this should be async)
    while (!device) {
        // Process callbacks
    }

    // Set up error callback
    device.SetUncapturedErrorCallback(
        [](WGPUErrorType type, const char* message, void*) {
            Log::Error(Event::Render, "WebGPU Error (%d): %s", type, message);
        },
        nullptr
    );
}

} // namespace webgpu
} // namespace mbgl