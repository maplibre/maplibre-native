#pragma once

#include <mbgl/gfx/shader_group.hpp>

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace mbgl {
namespace gfx {

using ShaderGroupPtr = std::shared_ptr<gfx::ShaderGroup>;

/// @brief A ShaderRegistry contains a collection of gfx::ShaderGroup instances.
/// Using the registry, shader groups may be dynamically registered or replaced
/// at runtime.
class ShaderRegistry {
public:
    explicit ShaderRegistry();
    ShaderRegistry(const ShaderRegistry&) = delete;
    ShaderRegistry(ShaderRegistry&&) noexcept = delete;
    ShaderRegistry& operator=(const ShaderRegistry&) = delete;
    ShaderRegistry& operator=(ShaderRegistry&&) noexcept = delete;
    virtual ~ShaderRegistry() = default;

    /// @brief Get the legacy shader group.
    /// @return A `gfx::ShaderGroup`
    [[nodiscard]] ShaderGroup& getLegacyGroup() noexcept;

    /// @brief Checks if a shader group exists in the registry for the given name.
    /// @param shaderGroupName Name of shader group
    /// @return If a shader group is found, true
    [[nodiscard]] virtual bool isShaderGroup(const std::string& shaderGroupName) const noexcept;

    /// @brief Get a shader group from the registry by name.
    /// @param shaderGroupName Name of shader group
    /// @return A `gfx::ShaderGroup` or `nullptr` if no shader group is found with the
    /// given name
    [[nodiscard]] virtual const ShaderGroupPtr getShaderGroup(const std::string& shaderGroupName) const noexcept;

    /// @brief Replace a matching shader group in the registry with the provided
    /// name.
    /// @param shaderGroup A `gfx::ShaderGroup`. The ShaderRegistry will take ownership.
    /// @param shaderGroupName Unique name to register the shader group under.
    /// @return True if a match was found and the shader group was replaced, false
    /// otherwise.
    [[nodiscard]] virtual bool replaceShader(ShaderGroupPtr&& shaderGroup, const std::string& shaderName) noexcept;

    /// @brief Register a new shader group with the registry for the given name.
    /// If a shader group is present in the registry with a conflicting name,
    /// registration will fail.
    /// @param shaderGroup A `gfx::ShaderGroup` to register. The ShaderRegistry will
    /// take ownership.
    /// @param shaderGroupName Unique name to register the shader group under.
    /// @return True if the shader group was registered, false if another shader group is
    /// already present with a conflicting name.
    [[nodiscard]] virtual bool registerShader(std::shared_ptr<Shader>&& shader, const std::string& shaderName) noexcept;
private:
    gfx::ShaderGroup legacyGroup;
    std::unordered_map<std::string, ShaderGroupPtr> shaderGroups;
    mutable std::shared_mutex shaderGroupLock;
};

} // namespace gfx
} // namespace mbgl
