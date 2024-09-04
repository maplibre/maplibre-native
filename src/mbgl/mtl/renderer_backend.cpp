#include <mbgl/mtl/renderer_backend.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>

#include <mbgl/shaders/mtl/shader_group.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/util/logging.hpp>

// ... shader_manifest.hpp
#include <mbgl/shaders/mtl/background.hpp>
#include <mbgl/shaders/mtl/background_pattern.hpp>
#include <mbgl/shaders/mtl/circle.hpp>
#include <mbgl/shaders/mtl/clipping_mask.hpp>
#include <mbgl/shaders/mtl/collision_box.hpp>
#include <mbgl/shaders/mtl/collision_circle.hpp>
#include <mbgl/shaders/mtl/custom_symbol_icon.hpp>
#include <mbgl/shaders/mtl/debug.hpp>
#include <mbgl/shaders/mtl/fill.hpp>
#include <mbgl/shaders/mtl/fill_extrusion.hpp>
#include <mbgl/shaders/mtl/fill_extrusion_pattern.hpp>
#include <mbgl/shaders/mtl/heatmap.hpp>
#include <mbgl/shaders/mtl/heatmap_texture.hpp>
#include <mbgl/shaders/mtl/hillshade.hpp>
#include <mbgl/shaders/mtl/hillshade_prepare.hpp>
#include <mbgl/shaders/mtl/line.hpp>
#include <mbgl/shaders/mtl/fill.hpp>
#include <mbgl/shaders/mtl/raster.hpp>
#include <mbgl/shaders/mtl/symbol_icon.hpp>
#include <mbgl/shaders/mtl/symbol_sdf.hpp>
#include <mbgl/shaders/mtl/symbol_text_and_icon.hpp>
#include <mbgl/shaders/mtl/widevector.hpp>

#include <cassert>
#include <string>

namespace mbgl {
namespace mtl {

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_),
      device(NS::TransferPtr(MTL::CreateSystemDefaultDevice())),
      commandQueue(NS::TransferPtr(device->newCommandQueue())) {
    assert(device);
    assert(commandQueue);
#if TARGET_OS_SIMULATOR
    baseVertexInstanceDrawingSupported = true;
#else
    baseVertexInstanceDrawingSupported = device->supportsFamily(MTL::GPUFamilyApple3);
#endif
}

RendererBackend::~RendererBackend() = default;

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    return std::make_unique<mtl::Context>(*this);
}

PremultipliedImage RendererBackend::readFramebuffer(const Size& size) {
    return PremultipliedImage(size);
}

void RendererBackend::assumeFramebufferBinding(const mtl::FramebufferID fbo) {}

void RendererBackend::assumeViewport(int32_t x, int32_t y, const Size& size) {}

void RendererBackend::assumeScissorTest(bool enabled) {}

bool RendererBackend::implicitFramebufferBound() {
    return false;
}

void RendererBackend::setFramebufferBinding(const mtl::FramebufferID fbo) {}

void RendererBackend::setViewport(int32_t x, int32_t y, const Size& size) {}

void RendererBackend::setScissorTest(bool enabled) {}

/// @brief Register a list of types with a shader registry instance
/// @tparam ...ShaderID Pack of BuiltIn:: shader IDs
/// @param registry A shader registry instance
/// @param programParameters ProgramParameters used to initialize each instance
template <shaders::BuiltIn... ShaderID>
void registerTypes(gfx::ShaderRegistry& registry, const ProgramParameters& programParameters) {
    /// The following fold expression will create a shader for every type
    /// in the parameter pack and register it with the shader registry.

    /// Registration calls are wrapped in a lambda that throws on registration
    /// failure, we shouldn't expect registration to faill unless the shader
    /// registry instance provided already has conflicting programs present.
    (
        [&]() {
            using namespace std::string_literals;
            using ShaderClass = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>;
            auto group = std::make_shared<ShaderGroup<ShaderID>>(programParameters);
            if (!registry.registerShaderGroup(std::move(group), ShaderClass::name)) {
                assert(!"duplicate shader group");
                throw std::runtime_error("Failed to register "s + ShaderClass::name + " with shader registry!");
            }
        }(),
        ...);
}

void RendererBackend::initShaders(gfx::ShaderRegistry& shaders, const ProgramParameters& programParameters) {
    registerTypes<shaders::BuiltIn::BackgroundShader,
                  shaders::BuiltIn::BackgroundPatternShader,
                  shaders::BuiltIn::CircleShader,
                  shaders::BuiltIn::ClippingMaskProgram,
                  shaders::BuiltIn::CollisionBoxShader,
                  shaders::BuiltIn::CollisionCircleShader,
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
                  shaders::BuiltIn::LineSDFShader,
                  shaders::BuiltIn::LinePatternShader,
                  shaders::BuiltIn::RasterShader,
                  shaders::BuiltIn::SymbolIconShader,
                  shaders::BuiltIn::SymbolSDFIconShader,
                  shaders::BuiltIn::SymbolTextAndIconShader,
                  shaders::BuiltIn::WideVectorShader>(shaders, programParameters);
}

} // namespace mtl
} // namespace mbgl
