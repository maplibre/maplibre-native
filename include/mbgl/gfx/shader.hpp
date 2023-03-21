#pragma once

#include <string_view>
#include <type_traits>

namespace mbgl {
namespace gfx {

/// @brief A shader is used as the base class for all programs across any supported
/// backend API. Shaders are registered with a `gfx::ShaderRegistry` instance.
class Shader {
    public:
        virtual ~Shader() = default;

        /// @brief Get the name of this shader
        /// @return Shader name
        virtual const std::string_view name() const noexcept = 0;
};

} // namespace gfx
} // namespace mbgl
