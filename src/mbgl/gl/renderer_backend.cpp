#include <mbgl/gl/renderer_backend.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/extension.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/util/logging.hpp>

#include <cassert>

namespace mbgl {
namespace gl {

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_) {}

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    auto result = std::make_unique<gl::Context>(*this);
    result->enableDebugging();
    result->initializeExtensions(std::bind(&RendererBackend::getExtensionFunctionPointer, this, std::placeholders::_1));
    return result;
}

PremultipliedImage RendererBackend::readFramebuffer(const Size& size) {
    return getContext<gl::Context>().readFramebuffer<PremultipliedImage>(size);
}

void RendererBackend::assumeFramebufferBinding(const gl::FramebufferID fbo) {
    getContext<gl::Context>().bindFramebuffer.setCurrentValue(fbo);
    if (fbo != ImplicitFramebufferBinding) {
        assert(gl::value::BindFramebuffer::Get() == getContext<gl::Context>().bindFramebuffer.getCurrentValue());
    }
}

void RendererBackend::assumeViewport(int32_t x, int32_t y, const Size& size) {
    getContext<gl::Context>().viewport.setCurrentValue({x, y, size});
    assert(gl::value::Viewport::Get() == getContext<gl::Context>().viewport.getCurrentValue());
}

void RendererBackend::assumeScissorTest(bool enabled) {
    getContext<gl::Context>().scissorTest.setCurrentValue(enabled);
    assert(gl::value::ScissorTest::Get() == getContext<gl::Context>().scissorTest.getCurrentValue());
}

bool RendererBackend::implicitFramebufferBound() {
    return getContext<gl::Context>().bindFramebuffer.getCurrentValue() == ImplicitFramebufferBinding;
}

void RendererBackend::setFramebufferBinding(const gl::FramebufferID fbo) {
    getContext<gl::Context>().bindFramebuffer = fbo;
    if (fbo != ImplicitFramebufferBinding) {
        assert(gl::value::BindFramebuffer::Get() == getContext<gl::Context>().bindFramebuffer.getCurrentValue());
    }
}

void RendererBackend::setViewport(int32_t x, int32_t y, const Size& size) {
    getContext<gl::Context>().viewport = {x, y, size};
    assert(gl::value::Viewport::Get() == getContext<gl::Context>().viewport.getCurrentValue());
}

void RendererBackend::setScissorTest(bool enabled) {
    getContext<gl::Context>().scissorTest = enabled;
    assert(gl::value::ScissorTest::Get() == getContext<gl::Context>().scissorTest.getCurrentValue());
}

RendererBackend::~RendererBackend() = default;

/// @brief Register a list of types with a shader registry instance
/// @tparam ...ShaderID Pack of BuiltIn:: shader IDs
/// @param registry A shader registry instance
/// @param glContext The GL context instance
/// @param programParameters ProgramParameters used to initialize each instance
template <shaders::BuiltIn... ShaderID>
void registerTypes(gfx::ShaderRegistry& registry, gl::Context& glContext, const ProgramParameters& programParameters) {
    /// The following fold expression will create a shader for every type
    /// in the parameter pack and register it with the shader registry.

    /// Registration calls are wrapped in a lambda that throws on registration
    /// failure, we shouldn't expect registration to faill unless the shader
    /// registry instance provided already has conflicting programs present.
    (
        [&](const std::string& name, const std::string& vert, const std::string& frag) {
            using Ty = shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>;
            if (!registry.registerShader(gl::ShaderProgramGL::create(glContext, programParameters, name, vert, frag),
                                         name)) {
                throw std::runtime_error("Failed to register " + std::string(Ty::name) + " with shader registry!");
            }
        }(shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::name,
          shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::vertex,
          shaders::ShaderSource<ShaderID, gfx::Backend::Type::OpenGL>::fragment),
        ...);
}

void RendererBackend::initShaders(gfx::ShaderRegistry& shaders, const ProgramParameters& programParameters) {
    registerTypes<shaders::BuiltIn::BackgroundShader,
                  shaders::BuiltIn::FillShader,
                  shaders::BuiltIn::FillOutlineShader,
                  shaders::BuiltIn::FillOutlinePatternShader,
                  shaders::BuiltIn::FillPatternShader>(shaders, static_cast<gl::Context&>(*context), programParameters);
}

} // namespace gl
} // namespace mbgl
