#pragma once

#include <mbgl/gfx/shader.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/util/hash.hpp>

#include <iomanip>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>

namespace mbgl {

using StringIDSetsPair = std::pair<unordered_set<std::string_view>, unordered_set<size_t>>;

namespace gfx {

class Context;

/// @brief A ShaderGroup contains a collection of gfx::Shader.
/// Using the group, shaders may be dynamically registered or replaced
/// at runtime.
class ShaderGroup {
public:
    ShaderGroup() = default;
    ShaderGroup(const ShaderGroup&) = delete;
    ShaderGroup(ShaderGroup&&) noexcept = delete;
    ShaderGroup& operator=(const ShaderGroup&) = delete;
    ShaderGroup& operator=(ShaderGroup&&) noexcept = delete;
    virtual ~ShaderGroup() = default;

    /// @brief Checks if a shader exists in the group for the given name.
    /// @param shaderName Name of shader
    /// @return If a shader is found, true
    [[nodiscard]] virtual bool isShader(const std::string& shaderName) const noexcept;

    /// @brief Get a shader from the group by name.
    /// @param shaderName Name of shader
    /// @return A `gfx::Shader` or `nullptr` if no shader is found with the
    /// given name
    [[nodiscard]] virtual const std::shared_ptr<gfx::Shader> getShader(const std::string& shaderName) const noexcept;

    /// @brief Replace a matching shader in the group with the provided
    /// instance. Shader type-names must match.
    /// @param shader A `gfx::Shader`. The ShaderGroup will take ownership.
    /// @return True if a match was found and the shader was replaced, false
    /// otherwise.
    [[nodiscard]] virtual bool replaceShader(std::shared_ptr<Shader>&& shader) noexcept;

    /// @brief Replace a matching shader in the group with the provided
    /// instance. Shader type-names must match.
    /// This variant replaces by explicit name.
    /// @param shader A `gfx::Shader`. The ShaderGroup will take ownership.
    /// @param shaderName Unique name to register the shader under.
    /// @return True if a match was found and the shader was replaced, false
    /// otherwise.
    [[nodiscard]] virtual bool replaceShader(std::shared_ptr<Shader>&& shader, const std::string& shaderName) noexcept;

    /// @brief Register a new shader with the group. If a shader is present
    /// in the group with a conflicting name, registration will fail.
    /// @param shader A `gfx::Shader` to register. The ShaderGroup will
    /// take ownership.
    /// @return True if the shader was registered, false if another shader is
    /// already present with a conflicting name.
    [[nodiscard]] virtual bool registerShader(std::shared_ptr<Shader>&& shader) noexcept;

    /// @brief Register a new shader with the group. If a shader is present
    /// in the group with a conflicting name, registration will fail.
    /// This variant registers using an explicit name.
    /// @param shader A `gfx::Shader` to register. The ShaderGroup will
    /// take ownership.
    /// @param shaderName Unique name to register the shader under.
    /// @return True if the shader was registered, false if another shader is
    /// already present with a conflicting name.
    [[nodiscard]] virtual bool registerShader(std::shared_ptr<Shader>&& shader, const std::string& shaderName) noexcept;

    /// @brief Shorthand helper to quickly get a derived type from the group.
    /// @tparam T Derived type, inheriting `gfx::Shader`
    /// @param shaderName The group name to look up
    /// @return T or nullptr if not found in the group
    template <typename T>
    std::shared_ptr<T> get(const std::string& shaderName) noexcept
        requires(is_shader_v<T>)
    {
        auto shader = getShader(shaderName);
        if (!shader || shader->typeName() != T::Name) {
            return nullptr;
        }
        return std::static_pointer_cast<T>(shader);
    }

    /// @brief Shorthand helper to quickly get a derived type from the group.
    /// This variant looks up shaders only by type name.
    /// @tparam T Derived type, inheriting `gfx::Shader`
    /// @return T or nullptr if not found in the group
    template <typename T>
    std::shared_ptr<T> get() noexcept
        requires(is_shader_v<T>)
    {
        auto shader = getShader(std::string(T::Name));
        if (!shader || shader->typeName() != T::Name) {
            return nullptr;
        }
        return std::static_pointer_cast<T>(shader);
    }

    /// @brief Ensure the destination 'to' is populated with the requested
    /// shader. If already non-null, does nothing.
    /// @tparam T Derived type, inheriting `gfx::Shader`
    /// @param to Location to store the shader
    /// @param shaderName The group name to look up
    /// @return True if 'to' has a valid program object, false otherwise.
    template <typename T>
    bool populate(std::shared_ptr<T>& to, const std::string& shaderName) noexcept
        requires(is_shader_v<T>)
    {
        if (to) {
            return true;
        }

        auto shader = getShader(shaderName);
        if (!shader || shader->typeName() != T::Name) {
            return false;
        }
        to = std::static_pointer_cast<T>(shader);
        return true;
    }

    /// @brief Ensure the destination 'to' is populated with the requested
    /// shader. If already non-null, does nothing. This variant looks up
    /// shaders only by type name.
    /// @tparam T Derived type, inheriting `gfx::Shader`
    /// @param to Location to store the shader
    /// @return True if 'to' has a valid program object, false otherwise.
    template <typename T>
    bool populate(std::shared_ptr<T>& to) noexcept
        requires(is_shader_v<T>)
    {
        if (to) {
            return true;
        }

        auto shader = getShader(std::string(T::Name));
        if (!shader || shader->typeName() != T::Name) {
            return false;
        }
        to = std::static_pointer_cast<T>(shader);
        return true;
    }

    /// @brief Get a shader from the group by its set of data driven properties as uniforms.
    /// if no shader is found, create and register the shader.
    /// @param propertiesAsUniforms Set of data driven properties as uniforms.
    /// @param firstAttribName Name of the first attribute
    /// @return A `gfx::ShaderPtr`
    virtual gfx::ShaderPtr getOrCreateShader(gfx::Context&,
                                             [[maybe_unused]] const StringIDSetsPair& propertiesAsUniforms,
                                             [[maybe_unused]] std::string_view firstAttribName = "a_pos") {
        return {};
    }

protected:
    using PropertyHashType = std::uint64_t;

    std::string getShaderName(const std::string_view& name, const PropertyHashType key) {
        return (std::ostringstream() << name << '#' << std::hex << key).str();
    }

    /// Generate a map key for the specified combination of properties
    PropertyHashType propertyHash(const StringIDSetsPair& propertiesAsUniforms) {
        const auto beg = propertiesAsUniforms.second.cbegin();
        const auto end = propertiesAsUniforms.second.cend();
        return util::order_independent_hash<decltype(beg), PropertyHashType>(beg, end);
    }

private:
    mbgl::unordered_map<std::string, std::shared_ptr<gfx::Shader>> programs;
    mutable std::shared_mutex programLock;
};

} // namespace gfx
} // namespace mbgl
