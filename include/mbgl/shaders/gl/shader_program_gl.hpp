#pragma once

#include <mbgl/gl/context.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/shaders/shader_program_base.hpp>

namespace mbgl {
namespace gl {

class ShaderProgramGL final : public gfx::ShaderProgramBase {
public:
    ShaderProgramGL(UniqueProgram&& glProgram_);
    ShaderProgramGL(ShaderProgramGL&& other);
    virtual ~ShaderProgramGL() noexcept = default;

    static constexpr std::string_view Name{"GenericGLShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    static std::shared_ptr<ShaderProgramGL> create(
        Context&,
        std::string_view name,
        std::string_view vertexSource,
        std::string_view fragmentSource) noexcept(false);
    
protected:
    UniqueProgram glProgram;
    //VertexArray vertexArray;
    //gfx::DrawScope drawScopeResource;
};

} // namespace gl
} // namespace mbgl
