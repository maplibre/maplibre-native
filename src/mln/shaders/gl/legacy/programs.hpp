#pragma once

#include <mln/shaders/program_parameters.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <memory>

namespace mln {

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

} // namespace mln
