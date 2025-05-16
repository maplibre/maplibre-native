#pragma once

#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <memory>

namespace mbgl {

class Programs {
public:
    Programs(const ProgramParameters&);
    ~Programs();

    /// @brief Registers built-in programs with the provided registry.
    /// @param registry gfx::ShaderRegistry to populate with built-in programs.
    void registerWith(gfx::ShaderRegistry& registry);

private:
    ProgramParameters programParameters;
};

} // namespace mbgl
