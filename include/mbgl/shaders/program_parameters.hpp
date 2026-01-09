#pragma once

#include <mbgl/gfx/backend.hpp>
#include <mbgl/shaders/shader_source.hpp>

#include <array>
#include <cassert>
#include <string>
#include <unordered_map>

namespace mbgl {

class ProgramParameters {
public:
    struct ProgramSource {
        gfx::Backend::Type backend{gfx::Backend::Type::TYPE_MAX};
        std::string vertex;
        std::string fragment;

        ProgramSource() = default;

        /// @brief Construct a new ProgramSource object
        /// @param forBackend The provided source code is intended for use with the given rendering backend.
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

    /// @brief Provide a `BuiltIn` value used to indicate the program type
    /// @param source ProgramSource
    /// @return Mutated ProgramParameters
    ProgramParameters withProgramType(shaders::BuiltIn type) const noexcept;

    shaders::BuiltIn getProgramType() const noexcept { return programType; }

    /// @brief Get a list of built-in shader preprocessor defines
    /// @return Shader source string
    /// @todo With the addition of future backends, defines should also be backend-aware
    const std::string& getDefinesString() const;

    /// @brief Get a list of built-in shader preprocessor defines
    /// @return Map of key-value pairs representing preprocessor definitions
    const std::unordered_map<std::string, std::string>& getDefines() const { return defines; }

    /// @brief Get source code for the vertex shader compatible with the requested backend
    /// @param backend Backend type
    /// @return Shader source string
    const std::string& vertexSource(gfx::Backend::Type backend) const;

    /// @brief Get source code for the fragment shader compatible with the requested backend
    /// @param backend Backend type
    /// @return Shader source string
    const std::string& fragmentSource(gfx::Backend::Type backend) const;

    bool getOverdrawInspectorEnabled() const { return overdrawInspector; }
    std::uint64_t getDefinesHash() const { return definesHash; }

private:
    std::unordered_map<std::string, std::string> defines;
    std::uint64_t definesHash;
    shaders::BuiltIn programType = shaders::BuiltIn::None;

    // cached value of `defines` converted to string format
    mutable std::string definesStr;

    std::array<ProgramSource, static_cast<size_t>(gfx::Backend::Type::TYPE_MAX)> defaultSources;
    std::array<ProgramSource, static_cast<size_t>(gfx::Backend::Type::TYPE_MAX)> userSources;

    bool overdrawInspector;
};

} // namespace mbgl
