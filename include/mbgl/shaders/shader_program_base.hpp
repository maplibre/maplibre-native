#pragma once

#include <mbgl/gfx/shader.hpp>
#include <mbgl/util/identity.hpp>

#include <string>

namespace mbgl {
namespace gfx {

class ShaderProgramBase : public gfx::Shader {
protected:
    ShaderProgramBase() { }
    ShaderProgramBase(ShaderProgramBase&&) { }
    
    const util::SimpleIdentity& getID() const { return shaderProgramID; }
    
public:
    virtual ~ShaderProgramBase() noexcept = default;
    
protected:
    util::SimpleIdentity shaderProgramID;
};

} // namespace gfx
} // namespace mbgl
