#pragma once

#include <mbgl/gl/context.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/shaders/shader_program_base.hpp>

namespace mbgl {
namespace gl {

class ShaderProgramGL final : public gfx::ShaderProgramBase {
public:
    ShaderProgramGL(UniqueProgram&& glProgram_);
    ShaderProgramGL(UniqueProgram&&, VertexAttributeArrayGL&& uniforms, VertexAttributeArrayGL&& attributes);
    ShaderProgramGL(ShaderProgramGL&& other);
    ~ShaderProgramGL() noexcept override = default;

    static constexpr std::string_view Name{"GenericGLShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    static std::shared_ptr<ShaderProgramGL> create(Context&,
                                                   std::string_view name,
                                                   std::string_view vertexSource,
                                                   std::string_view fragmentSource) noexcept(false);

    const gfx::VertexAttributeArray& getUniforms() const override { return uniforms; }

    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }

    ProgramID getGLProgramID() const { return glProgram; }

protected:
    gfx::VertexAttributeArray& mutableUniforms() override { return uniforms; }

    gfx::VertexAttributeArray& mutableVertexAttributes() override { return vertexAttributes; }

protected:
    UniqueProgram glProgram;

    VertexAttributeArrayGL uniforms;
    VertexAttributeArrayGL vertexAttributes;
};

} // namespace gl
} // namespace mbgl
