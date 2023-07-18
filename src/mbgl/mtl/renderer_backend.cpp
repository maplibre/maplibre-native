#include <mbgl/mtl/renderer_backend.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/shaders/mtl/shader_group.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/util/logging.hpp>

// metal-cpp is a header-only library.
// To generate the implementation,
// add the following code in one of your .cpp files:
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <cassert>

namespace mbgl {
namespace mtl {

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_),
    device(MTL::CreateSystemDefaultDevice()) {
    assert(device);
}

RendererBackend::~RendererBackend() {
    if (device) {
        device->release();
        device = nullptr;
    }
}

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    return std::make_unique<mtl::Context>(*this);
}

PremultipliedImage RendererBackend::readFramebuffer(const Size& size) {
    return PremultipliedImage(size);
    // return getContext<mtl::Context>().readFramebuffer<PremultipliedImage>(size);
}

void RendererBackend::assumeFramebufferBinding(const mtl::FramebufferID fbo) {
    /*getContext<mtl::Context>().bindFramebuffer.setCurrentValue(fbo);
    if (fbo != ImplicitFramebufferBinding) {
        assert(mtl::value::BindFramebuffer::Get() == getContext<mtl::Context>().bindFramebuffer.getCurrentValue());
    }*/
}

void RendererBackend::assumeViewport(int32_t x, int32_t y, const Size& size) {
    /*getContext<mtl::Context>().viewport.setCurrentValue({x, y, size});
    assert(mtl::value::Viewport::Get() == getContext<mtl::Context>().viewport.getCurrentValue());*/
}

void RendererBackend::assumeScissorTest(bool enabled) {
    /*getContext<mtl::Context>().scissorTest.setCurrentValue(enabled);
    assert(mtl::value::ScissorTest::Get() == getContext<mtl::Context>().scissorTest.getCurrentValue());*/
}

bool RendererBackend::implicitFramebufferBound() {
    return false;
    // return getContext<mtl::Context>().bindFramebuffer.getCurrentValue() == ImplicitFramebufferBinding;
}

void RendererBackend::setFramebufferBinding(const mtl::FramebufferID fbo) {
    /*getContext<mtl::Context>().bindFramebuffer = fbo;
    if (fbo != ImplicitFramebufferBinding) {
        assert(mtl::value::BindFramebuffer::Get() == getContext<mtl::Context>().bindFramebuffer.getCurrentValue());
    }*/
}

void RendererBackend::setViewport(int32_t x, int32_t y, const Size& size) {
    /*getContext<mtl::Context>().viewport = {x, y, size};
    assert(mtl::value::Viewport::Get() == getContext<mtl::Context>().viewport.getCurrentValue());*/
}

void RendererBackend::setScissorTest(bool enabled) {
    /*getContext<mtl::Context>().scissorTest = enabled;
    assert(mtl::value::ScissorTest::Get() == getContext<mtl::Context>().scissorTest.getCurrentValue());*/
}

#if MLN_DRAWABLE_RENDERER
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
    /*(
        [&]() {
            const auto name = std::string(shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>::name);
            if (!registry.registerShaderGroup(std::make_shared<ShaderGroup<ShaderID>>(programParameters), name)) {
                throw std::runtime_error("Failed to register " + name + " with shader registry!");
            }
        }(),
    ...);*/
}

void RendererBackend::initShaders(gfx::ShaderRegistry& shaders, const ProgramParameters& programParameters) {
    registerTypes<shaders::BuiltIn::BackgroundShader,
                  shaders::BuiltIn::BackgroundPatternShader,
                  shaders::BuiltIn::CircleShader,
                  shaders::BuiltIn::FillShader,
                  shaders::BuiltIn::FillOutlineShader,
                  shaders::BuiltIn::LineShader,
                  shaders::BuiltIn::LineSDFShader,
                  shaders::BuiltIn::LinePatternShader,
                  shaders::BuiltIn::LineGradientShader,
                  shaders::BuiltIn::FillOutlinePatternShader,
                  shaders::BuiltIn::FillPatternShader,
                  shaders::BuiltIn::FillExtrusionShader,
                  shaders::BuiltIn::FillExtrusionPatternShader,
                  shaders::BuiltIn::HeatmapShader,
                  shaders::BuiltIn::HeatmapTextureShader,
                  shaders::BuiltIn::HillshadePrepareShader,
                  shaders::BuiltIn::HillshadeShader,
                  shaders::BuiltIn::RasterShader,
                  shaders::BuiltIn::SymbolIconShader,
                  shaders::BuiltIn::SymbolSDFTextShader,
                  shaders::BuiltIn::SymbolSDFIconShader,
                  shaders::BuiltIn::SymbolTextAndIconShader>(shaders, programParameters);
}
#endif

} // namespace mtl
} // namespace mbgl
