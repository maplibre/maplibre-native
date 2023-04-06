#pragma once

#include <mbgl/gfx/shader.hpp>
#include <mbgl/util/identity.hpp>

#include <string>

namespace mbgl {

class ShaderProgramBase : public gfx::Shader {
protected:
    ShaderProgramBase() { }
    ShaderProgramBase(ShaderProgramBase&&) { }

    const util::SimpleIdentity& getID() const { return shaderProgramID; }

public:
    virtual ~ShaderProgramBase() = default;

protected:
    util::SimpleIdentity shaderProgramID;
};

} // namespace mbgl
