#pragma once

#include <string_view>
#include <type_traits>

namespace mbgl {
namespace gfx {

class Shader;

// Assert that a type is a valid shader for downcasting.
// A valid shader must:
//   * Inherit gfx::Shader
//   * Declare a public, unique type name (string_view T::Name)
//   * Be a final class
template <typename T>
inline constexpr bool is_shader_v = std::is_base_of_v<gfx::Shader, T> &&
                                    std::is_same_v<std::remove_cv_t<decltype(T::Name)>, std::string_view> &&
                                    std::is_final_v<T>;

/// @brief A shader is used as the base class for all programs across any supported
/// backend API. Shaders are registered with a `gfx::ShaderRegistry` instance.
class Shader {
public:
    virtual ~Shader() = default;

    /// @brief Get the type name of this shader
    /// @return Shader type name
    virtual const std::string_view typeName() const noexcept = 0;

    /// @brief Downcast to a type
    /// @tparam T Derived type
    /// @return Type or nullptr if type info was not a match
    template <typename T, typename std::enable_if_t<is_shader_v<T>, bool>* = nullptr>
    T* to() noexcept {
        if (typeName() != T::Name) {
            return nullptr;
        }
        return static_cast<T*>(this);
    }
};

} // namespace gfx
} // namespace mbgl
