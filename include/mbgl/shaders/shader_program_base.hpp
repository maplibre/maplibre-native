#pragma once

#include <mbgl/gfx/shader.hpp>

#include <string>

namespace mbgl {

class ShaderProgramBase : public gfx::Shader {
protected:
    ShaderProgramBase() { }

public:
    virtual ~ShaderProgramBase() = default;

protected:
};

} // namespace mbgl
