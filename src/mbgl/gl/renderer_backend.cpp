#include <mbgl/gl/renderer_backend.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/extension.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#include <mbgl/shaders/gl/shader_group_gl.hpp>
#include <mbgl/shaders/gl/legacy/programs.hpp>

#include <cassert>

namespace mbgl {
namespace gl {

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_) {}

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_, const TaggedScheduler& threadPool_)
    : gfx::RendererBackend(contextMode_, threadPool_) {}

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    MLN_TRACE_FUNC();

    auto result = std::make_unique<gl::Context>(
        *this); // Tagged background thread pool will be owned by the RendererBackend
    result->enableDebugging();
    result->initializeExtensions(std::bind(&RendererBackend::getExtensionFunctionPointer, this, std::placeholders::_1));
    return result;
}

PremultipliedImage RendererBackend::readFramebuffer(const Size& size) {
    MLN_TRACE_FUNC();

    return getContext<gl::Context>().readFramebuffer<PremultipliedImage>(size);
}

void RendererBackend::assumeFramebufferBinding(const gl::FramebufferID fbo) {
    MLN_TRACE_FUNC();

    getContext<gl::Context>().bindFramebuffer.setCurrentValue(fbo);
    if (fbo != ImplicitFramebufferBinding) {
        assert(gl::value::BindFramebuffer::Get() == getContext<gl::Context>().bindFramebuffer.getCurrentValue());
    }
}

void RendererBackend::assumeViewport(int32_t x, int32_t y, const Size& size) {
    MLN_TRACE_FUNC();

    getContext<gl::Context>().viewport.setCurrentValue({x, y, size});
    assert(gl::value::Viewport::Get() == getContext<gl::Context>().viewport.getCurrentValue());
}

void RendererBackend::assumeScissorTest(int32_t x, int32_t y, uint32_t width, uint32_t height) {
    MLN_TRACE_FUNC();

    getContext<gl::Context>().scissorTest.setCurrentValue({x, y, width, height});
    assert(gl::value::ScissorTest::Get() == getContext<gl::Context>().scissorTest.getCurrentValue());
}

bool RendererBackend::implicitFramebufferBound() {
    MLN_TRACE_FUNC();

    return getContext<gl::Context>().bindFramebuffer.getCurrentValue() == ImplicitFramebufferBinding;
}

void RendererBackend::setFramebufferBinding(const gl::FramebufferID fbo) {
    MLN_TRACE_FUNC();

    getContext<gl::Context>().bindFramebuffer = fbo;
    if (fbo != ImplicitFramebufferBinding) {
        assert(gl::value::BindFramebuffer::Get() == getContext<gl::Context>().bindFramebuffer.getCurrentValue());
    }
}

void RendererBackend::setViewport(int32_t x, int32_t y, const Size& size) {
    MLN_TRACE_FUNC();

    getContext<gl::Context>().viewport = {x, y, size};
    assert(gl::value::Viewport::Get() == getContext<gl::Context>().viewport.getCurrentValue());
}

void RendererBackend::setScissorTest(int32_t x, int32_t y, uint32_t width, uint32_t height) {
    MLN_TRACE_FUNC();

    getContext<gl::Context>().scissorTest = {x, y, width, height};
    assert(gl::value::ScissorTest::Get() == getContext<gl::Context>().scissorTest.getCurrentValue());
}

RendererBackend::~RendererBackend() = default;

/// @brief Register a list of types with a shader registry instance
/// @tparam ...ShaderID Pack of BuiltIn:: shader IDs
/// @param registry A shader registry instance
/// @param programParameters ProgramParameters used to initialize each instance
template <shaders::BuiltIn... ShaderID>
void registerTypes(gfx::ShaderRegistry& registry, const ProgramParameters& programParameters) {
    MLN_TRACE_FUNC();

    /// The following fold expression will create a shader for every type
    /// in the parameter pack and register it with the shader registry.

    /// Registration calls are wrapped in a lambda that throws on registration
    /// failure, we shouldn't expect registration to faill unless the shader
    /// registry instance provided already has conflicting programs present.
    (
        [&]() {
            const auto name = std::string(shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::name);
            if (!registry.registerShaderGroup(std::make_shared<ShaderGroupGL<ShaderID>>(programParameters), name)) {
                throw std::runtime_error("Failed to register " + name + " with shader registry!");
            }
        }(),
        ...);
}

void RendererBackend::initShaders(gfx::ShaderRegistry& shaders, const ProgramParameters& programParameters) {
    MLN_TRACE_FUNC();

    registerTypes<shaders::BuiltIn::BackgroundShader,
                  shaders::BuiltIn::BackgroundPatternShader,
                  shaders::BuiltIn::CircleShader,
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
                  shaders::BuiltIn::HillshadePrepareShader,
                  shaders::BuiltIn::HillshadeShader,
                  shaders::BuiltIn::LineShader,
                  shaders::BuiltIn::LineGradientShader,
                  shaders::BuiltIn::LinePatternShader,
                  shaders::BuiltIn::LineSDFShader,
                  shaders::BuiltIn::LocationIndicatorShader,
                  shaders::BuiltIn::LocationIndicatorTexturedShader,
                  shaders::BuiltIn::RasterShader,
                  shaders::BuiltIn::SymbolIconShader,
                  shaders::BuiltIn::SymbolSDFShader,
                  shaders::BuiltIn::SymbolTextAndIconShader>(shaders, programParameters);

    // Initialize legacy shader programs
    Programs programs(programParameters);
    programs.registerWith(shaders);
}

} // namespace gl
} // namespace mbgl
