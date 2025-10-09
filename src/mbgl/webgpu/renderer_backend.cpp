#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/size.hpp>

// Include shader group and individual shader headers
#include <mbgl/shaders/webgpu/shader_group.hpp>
#include <mbgl/shaders/webgpu/background.hpp>
#include <mbgl/shaders/webgpu/circle.hpp>
#include <mbgl/shaders/webgpu/clipping_mask.hpp>
#include <mbgl/shaders/webgpu/collision.hpp>
#include <mbgl/shaders/webgpu/custom_geometry.hpp>
#include <mbgl/shaders/webgpu/custom_symbol_icon.hpp>
#include <mbgl/shaders/webgpu/debug.hpp>
#include <mbgl/shaders/webgpu/fill.hpp>
#include <mbgl/shaders/webgpu/fill_extrusion.hpp>
#include <mbgl/shaders/webgpu/heatmap.hpp>
#include <mbgl/shaders/webgpu/heatmap_texture.hpp>
#include <mbgl/shaders/webgpu/hillshade.hpp>
#include <mbgl/shaders/webgpu/hillshade_prepare.hpp>
#include <mbgl/shaders/webgpu/line.hpp>
#include <mbgl/shaders/webgpu/location_indicator.hpp>
#include <mbgl/shaders/webgpu/raster.hpp>
#include <mbgl/shaders/webgpu/symbol.hpp>
#include <mbgl/shaders/webgpu/widevector.hpp>

namespace mbgl {
namespace webgpu {

// Forward declare and define the Impl class
class RendererBackend::Impl {
public:
    void* instance = nullptr;
    void* device = nullptr;
    void* queue = nullptr;
    void* surface = nullptr;
    wgpu::TextureFormat depthStencilFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat colorFormat = wgpu::TextureFormat::Undefined;
};

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_),
      impl(std::make_unique<Impl>()) {}

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
    (
        [&]() {
            using ShaderClass = shaders::ShaderSource<ShaderID, gfx::Backend::Type::WebGPU>;
            auto group = std::make_shared<webgpu::ShaderGroup<ShaderID>>(programParameters);
            if (!registry.registerShaderGroup(std::move(group), ShaderClass::name)) {
                assert(!"duplicate shader group");
                throw std::runtime_error("Failed to register "s + ShaderClass::name + " with shader registry!");
            }
        }(),
        ...);
}

} // namespace

void RendererBackend::initShaders(gfx::ShaderRegistry& registry, const ProgramParameters& parameters) {
    // Register all shader types - even if headers don't exist yet, fallback implementation will be used
    // As WebGPU shader headers are created, they will automatically be picked up
    registerTypes<shaders::BuiltIn::BackgroundShader,
                  shaders::BuiltIn::BackgroundPatternShader,
                  shaders::BuiltIn::CircleShader,
                  shaders::BuiltIn::ClippingMaskProgram,
                  shaders::BuiltIn::CollisionBoxShader,
                  shaders::BuiltIn::CollisionCircleShader,
                  shaders::BuiltIn::CustomGeometryShader,
                  shaders::BuiltIn::CustomSymbolIconShader,
                  shaders::BuiltIn::DebugShader,
                  shaders::BuiltIn::FillShader,
                  shaders::BuiltIn::FillOutlineShader,
                  shaders::BuiltIn::FillPatternShader,
                  shaders::BuiltIn::FillOutlinePatternShader,
                  shaders::BuiltIn::FillOutlineTriangulatedShader,
                  shaders::BuiltIn::FillExtrusionShader,
                  shaders::BuiltIn::FillExtrusionPatternShader,
                  shaders::BuiltIn::HeatmapShader,
                  shaders::BuiltIn::HeatmapTextureShader,
                  shaders::BuiltIn::HillshadeShader,
                  shaders::BuiltIn::HillshadePrepareShader,
                  shaders::BuiltIn::LineShader,
                  shaders::BuiltIn::LineGradientShader,
                  shaders::BuiltIn::LinePatternShader,
                  shaders::BuiltIn::LineSDFShader,
                  shaders::BuiltIn::LocationIndicatorShader,
                  shaders::BuiltIn::LocationIndicatorTexturedShader,
                  shaders::BuiltIn::RasterShader,
                  shaders::BuiltIn::SymbolIconShader,
                  shaders::BuiltIn::SymbolSDFShader,
                  shaders::BuiltIn::SymbolTextAndIconShader,
                  shaders::BuiltIn::WideVectorShader>(registry, parameters);
}

void RendererBackend::setSurface(void* nativeWindow) {
    // Platform-specific surface creation will be handled by subclasses
    // For now, just store the native window handle
    // The actual surface creation depends on the platform (X11, Wayland, etc.)
    (void)nativeWindow;
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

void RendererBackend::setDepthStencilFormat(wgpu::TextureFormat format) {
    impl->depthStencilFormat = format;
}

wgpu::TextureFormat RendererBackend::getDepthStencilFormat() const {
    return impl->depthStencilFormat;
}

void RendererBackend::setColorFormat(wgpu::TextureFormat format) {
    impl->colorFormat = format;
}

wgpu::TextureFormat RendererBackend::getColorFormat() const {
    return impl->colorFormat;
}

} // namespace webgpu
} // namespace mbgl
