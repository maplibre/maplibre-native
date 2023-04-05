#pragma once

#include <mbgl/shaders/shader_program_base.hpp>

#include <string>

namespace mbgl {
namespace gl {

class ShaderProgramGL final : public ShaderProgramBase {
public:
    ShaderProgramGL() : ShaderProgramBase() { }
    virtual ~ShaderProgramGL() = default;

    static constexpr std::string_view Name{"GenericGLShader"};
    const std::string_view name() const noexcept override { return Name; }

    bool compile(std::string_view vert, std::string_view frag);
    
protected:
};

} // namespace gl
} // namespace mbgl
