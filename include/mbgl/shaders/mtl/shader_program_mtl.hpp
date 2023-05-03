#pragma once

#include <mbgl/shaders/shader_program_base.hpp>

namespace mbgl {
namespace mtl {

class ShaderProgramMTL : public gfx::ShaderProgramBase {
public:
    ShaderProgramMTL()
        : ShaderProgramBase() {}
    ~ShaderProgramMTL() noexcept override = default;

    static constexpr std::string_view Name{"GenericMTLShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    // bool compile(std::string_view vert, std::string_view frag);
    //  Load compiled Metal functions from the default library in a bundle
    // bool load(NSBundle*, std::string_view vertFuncName, std::string_view fragFuncName);
    //  Load compiled Metal functions from a specific library
    // bool load(id<MTLLibrary>, std::string_view vertFuncName, std::string_view fragFuncName);

protected:
};

} // namespace mtl
} // namespace mbgl
