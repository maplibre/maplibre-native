#include <mbgl/gl/renderer_backend.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/extension.hpp>
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

void RendererBackend::initShaders(gfx::ShaderRegistry& shaders) {
    constexpr auto shaderName = "background_generic";

    gl::Context& glContext = static_cast<gl::Context&>(*context);

    auto shader = shaders.get<gl::ShaderProgramGL>(shaderName);
    if (!shader) {
        constexpr auto vert = R"(#version 300 es
            precision highp float;
            
            uniform mat4 u_matrix;
            layout (location = 0) in vec2 a_pos;
            layout (location = 1) in vec4 a_color;
            out vec4 v_color;

            void main() {
                v_color = a_color;
                gl_Position = u_matrix * vec4(a_pos, 0, 1);
            })";
        constexpr auto frag = R"(#version 300 es
            precision highp float;
            in vec4 v_color;
            out vec4 fragColor;

            void main() {
                fragColor = v_color;
            })";

        try {
            // Compile
            shader = gl::ShaderProgramGL::create(glContext, shaderName, vert, frag);
            if (shader) {
                // Set default values
                //shader->setAttribute("a_color", gfx::VertexAttribute::matf4{ 1.0f, 1.0f, 1.0f, 1.0f });

                // Add to the registry
                if (!shaders.registerShader(shader, shaderName)) {
                    Log::Warning(Event::General, "Shader conflict - " + std::string(shaderName));
                    return;
                }
            } else {
                Log::Warning(Event::General, "Shader create failed - " + std::string(shaderName));
                return;
            }
    } catch (const std::runtime_error& ex) {
            Log::Warning(Event::General, "Shader create exception - " + std::string(ex.what()));
            return;
        }
    }
}

} // namespace gl
} // namespace mbgl
