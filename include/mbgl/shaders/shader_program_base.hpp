#pragma once

#include <mbgl/gfx/shader.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/util/identity.hpp>

#include <string>

namespace mbgl {
namespace gfx {

class ShaderProgramBase : public gfx::Shader {
protected:
    ShaderProgramBase() { }
    ShaderProgramBase(ShaderProgramBase&&) { }

public:
    virtual ~ShaderProgramBase() noexcept = default;

    const util::SimpleIdentity& getID() const { return shaderProgramID; }

    /// Get the available vertex attributes and their default values
    virtual const gfx::VertexAttributeArray& getVertexAttributes() const = 0;

protected:
    util::SimpleIdentity shaderProgramID;
};

} // namespace gfx
} // namespace mbgl
