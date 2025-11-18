#pragma once

#include <mbgl/gfx/shader.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/util/identity.hpp>

#include <string>
#include <optional>

namespace mbgl {
namespace gfx {

class ShaderProgramBase : public gfx::Shader {
protected:
    ShaderProgramBase() {}
    ShaderProgramBase(ShaderProgramBase&&) {}
    ~ShaderProgramBase() noexcept override = default;

    template <typename T>
    bool set(gfx::VertexAttributeArray& attrs, const size_t id, std::size_t i, T value) {
        const auto& item = attrs.get(id);
        if (item && i < item->getCount()) {
            item->set(i, value);
            return true;
        }
        return false;
    }

public:
    const util::SimpleIdentity& getID() const { return shaderProgramID; }

    /// @brief Gets the sampler location
    /// @param name uniform name
    virtual std::optional<size_t> getSamplerLocation(const size_t) const = 0;

    /// Get the available vertex attributes and their default values
    virtual const gfx::VertexAttributeArray& getVertexAttributes() const = 0;

    /// Get the available instance attributes and their default values
    virtual const gfx::VertexAttributeArray& getInstanceAttributes() const = 0;

protected:
    util::SimpleIdentity shaderProgramID;
};

} // namespace gfx
} // namespace mbgl
