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

    template <typename T>
    bool set(gfx::VertexAttributeArray& attrs, const std::string& name, std::size_t i, T value) {
        auto *item = attrs.get(name);
        if (item && i < item->getCount()) {
            item->set(i, value);
            return true;
        }
        return false;
    }

public:
    virtual ~ShaderProgramBase() noexcept = default;

    const util::SimpleIdentity& getID() const { return shaderProgramID; }

    /// Get the available vertex attributes and their default values
    virtual const gfx::VertexAttributeArray& getUniforms() const = 0;

    /// Get the available vertex attributes and their default values
    virtual const gfx::VertexAttributeArray& getVertexAttributes() const = 0;

    // Set a value if the element is present
    template <typename T>
    bool setUniform(const std::string& name, std::size_t i, T value) {
        return set(mutableUniforms(), name, i, value);
    }
    template <typename T>
    bool setAttribute(const std::string& name, std::size_t i, T value) {
        return set(mutableVertexAttributes(), name, i, value);
    }

    void updateUniforms() { mutableUniforms().applyUniforms(*this); }

protected:
    virtual gfx::VertexAttributeArray& mutableUniforms() = 0;
    virtual gfx::VertexAttributeArray& mutableVertexAttributes() = 0;

protected:
    util::SimpleIdentity shaderProgramID;
};

} // namespace gfx
} // namespace mbgl
