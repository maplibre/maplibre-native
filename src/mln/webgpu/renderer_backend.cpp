#include <mln/webgpu/renderer_backend.hpp>
#include <mln/webgpu/context.hpp>
#include <mln/gfx/renderable.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/size.hpp>

// Include shader group and individual shader headers
#include <mln/shaders/webgpu/shader_group.hpp>
#include <mln/shaders/webgpu/background.hpp>
#include <mln/shaders/webgpu/circle.hpp>
#include <mln/shaders/webgpu/clipping_mask.hpp>
#include <mln/shaders/webgpu/collision.hpp>
#include <mln/shaders/webgpu/custom_geometry.hpp>
#include <mln/shaders/webgpu/custom_symbol_icon.hpp>
#include <mln/shaders/webgpu/debug.hpp>
#include <mln/shaders/webgpu/fill.hpp>
#include <mln/shaders/webgpu/fill_extrusion.hpp>
#include <mln/shaders/webgpu/heatmap.hpp>
#include <mln/shaders/webgpu/heatmap_texture.hpp>
#include <mln/shaders/webgpu/hillshade.hpp>
#include <mln/shaders/webgpu/hillshade_prepare.hpp>
#include <mln/shaders/webgpu/color_relief.hpp>
#include <mln/shaders/webgpu/line.hpp>
#include <mln/shaders/webgpu/location_indicator.hpp>
#include <mln/shaders/webgpu/raster.hpp>
#include <mln/shaders/webgpu/symbol.hpp>
#include <mln/shaders/webgpu/widevector.hpp>

namespace mln {
namespace webgpu {

// Forward declare and define the Impl class
class RendererBackend::Impl {
public:
    void* instance = nullptr;
    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;
    void* surface = nullptr;
    wgpu::TextureFormat depthStencilFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat colorFormat = wgpu::TextureFormat::Undefined;
};

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_),
      impl(std::make_unique<Impl>()) {}

RendererBackend::~RendererBackend() {
    context.reset();
}

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
                  shaders::BuiltIn::ColorReliefShader,
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

void RendererBackend::setDevice(WGPUDevice device) {
    impl->device = device;
}

void RendererBackend::setQueue(WGPUQueue queue) {
    impl->queue = queue;
}

void* RendererBackend::getInstance() const {
    return impl->instance;
}

WGPUDevice RendererBackend::getDevice() const {
    return impl->device;
}

WGPUQueue RendererBackend::getQueue() const {
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

mln::Size RendererBackend::getFramebufferSize() const {
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
} // namespace mln
