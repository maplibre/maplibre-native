#pragma once

#include <mbgl/gfx/shader.hpp>

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace mbgl {
namespace gfx {

/// @brief A ShaderRegistry contains a collection of gfx::Shader instances.
/// Using the registry, shaders may be dynamically registered or replaced
/// at runtime.
class ShaderRegistry {
    public:
        explicit ShaderRegistry();
        ShaderRegistry(const ShaderRegistry&) = delete;
        ShaderRegistry(ShaderRegistry&&) noexcept = delete;
        ShaderRegistry& operator=(const ShaderRegistry&) = delete;
        ShaderRegistry& operator=(ShaderRegistry&&) noexcept = delete;
        virtual ~ShaderRegistry() = default;

        /// @brief Checks if a shader exists in the registry for the given name.
        /// @param shaderName Name of shader
        /// @return If a shader is found, true
        [[nodiscard]] virtual bool isShader(const std::string& shaderName)
            const noexcept;
        
        /// @brief Get a shader from the registry by name.
        /// @param shaderName Name of shader
        /// @return A `gfx::Shader` or `nullptr` if no shader is found with the
        /// given name
        [[nodiscard]] virtual const std::shared_ptr<gfx::Shader> getShader(
            const std::string& shaderName) const noexcept;

        /// @brief Replace a matching shader in the registry with the provided
        /// instance. Shader names must match.
        /// @param shader A `gfx::Shader`. The ShaderRegistry will take ownership.
        /// @return True if a match was found and the shader was replaced, false
        /// otherwise.
        [[nodiscard]] virtual bool replaceShader(
            std::shared_ptr<Shader>&& shader) noexcept;

        /// @brief Register a new shader with the registry. If a shader is present
        /// in the registry with a conflicting name, registration will fail.
        /// @param shader A `gfx::Shader` to register. The ShaderRegistry will
        /// take ownership.
        /// @return True if the shader was registered, false if another shader is
        /// already present with a conflicting name.
        [[nodiscard]] virtual bool registerShader(
            std::shared_ptr<Shader>&& shader) noexcept;

        /// @brief Shorthand helper to quickly get a derived type from the registry.
        /// @tparam T Derived type, inheriting `gfx::Shader`
        /// @return T or nullptr if not found in the registry
        template<typename T,
            typename std::enable_if_t<is_shader_v<T>, bool>* = nullptr>
        std::shared_ptr<T> get() noexcept {
            auto shader = getShader(std::string(T::Name));
            if (!shader || (shader->name() != T::Name)) {
                return nullptr;
            }
            return std::static_pointer_cast<T>(shader);
        }

    private:
        std::unordered_map<
            std::string,
            std::shared_ptr<gfx::Shader>
        > programs;
        mutable std::shared_mutex programLock;
};

} // namespace gfx
} // namespace mbgl
