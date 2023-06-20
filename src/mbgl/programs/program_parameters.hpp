#pragma once

#include <mbgl/gfx/backend.hpp>

#include <array>
#include <cassert>
#include <string>

namespace mbgl {

class ProgramParameters {
public:
    struct ProgramSource {
        gfx::Backend::Type backend{gfx::Backend::Type::TYPE_MAX};
        std::string vertex;
        std::string fragment;

        ProgramSource() = default;

        /// @brief Construct a new ProgramSource object
        /// @param forBackend The provided source code is intended for use with the give
        /// rendering backend.
        /// @param vertex_ The vertex shader source code, or empty string if not provided.
        /// @param fragment_ The fragment shader source code, or empty string if not provided.
        ProgramSource(gfx::Backend::Type forBackend, const std::string& vertex_, const std::string& fragment_)
            : backend(forBackend),
              vertex(vertex_),
              fragment(fragment_) {
            assert(gfx::Backend::Type::TYPE_MAX != forBackend);
        }
    };

    ProgramParameters(float pixelRatio, bool overdraw);

    /// @brief Provide custom shader code which overrides any default source present
    /// @param source ProgramSource
    /// @return Mutated ProgramParameters
    ProgramParameters withShaderSource(const ProgramSource& source) const noexcept;

    /// @brief Provide default shader source
    /// @param source ProgramSource
    /// @return Mutated ProgramParameters
    ProgramParameters withDefaultSource(const ProgramSource& source) const noexcept;

    /// @brief Get a list of built-in shader preprocessor defines
    /// @return Shader source string
    /// @todo With the addition of future backends, defines should also be backend-aware
    const std::string& getDefines() const;

    /// @brief Get source code for the vertex shader compatible with the requested backend
    /// @param backend Backend type
    /// @return Shader source string
    const std::string& vertexSource(gfx::Backend::Type backend) const;

    /// @brief Get source code for the fragment shader compatible with the requested backend
    /// @param backend Backend type
    /// @return Shader source string
    const std::string& fragmentSource(gfx::Backend::Type backend) const;

private:
    std::string defines;

    std::array<ProgramSource, static_cast<size_t>(gfx::Backend::Type::TYPE_MAX)> defaultSources;
    std::array<ProgramSource, static_cast<size_t>(gfx::Backend::Type::TYPE_MAX)> userSources;
};

} // namespace mbgl
