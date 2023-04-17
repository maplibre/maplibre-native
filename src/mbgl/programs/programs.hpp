#pragma once

#include <mbgl/programs/clipping_mask_program.hpp>
#include <mbgl/programs/debug_program.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <memory>

namespace mbgl {

class BackgroundLayerPrograms;

class CircleLayerPrograms;
class RasterLayerPrograms;
class HeatmapLayerPrograms;
class HillshadeLayerPrograms;
class FillLayerPrograms;
class FillExtrusionLayerPrograms;
class LineLayerPrograms;
class SymbolLayerPrograms;

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
