#include <mbgl/programs/programs.hpp>

#include <mbgl/programs/background_program.hpp>
#include <mbgl/programs/circle_program.hpp>
#include <mbgl/programs/heatmap_program.hpp>
#include <mbgl/programs/hillshade_program.hpp>
#include <mbgl/programs/fill_extrusion_program.hpp>
#include <mbgl/programs/fill_program.hpp>
#include <mbgl/programs/line_program.hpp>
#include <mbgl/programs/raster_program.hpp>
#include <mbgl/programs/symbol_program.hpp>

#include <mbgl/util/logging.hpp>
#include <exception>

namespace mbgl {

Programs::Programs(gfx::Context& context_, const ProgramParameters& programParameters_)
    : context(context_),
      programParameters(programParameters_) {}

Programs::~Programs() = default;

template<typename ... T>
void registerTypes(gfx::ShaderRegistry& registry,
    const ProgramParameters& programParameters_)
{
    ( [](bool expr) {
        if (!expr) {
            throw std::runtime_error(
                "Failed to register " +
                std::string(T::Name) +
                " with shader registry!");
        }
    }( registry.registerShader(
        std::make_shared<T>(programParameters_))), ... );
}

void Programs::registerWith([[maybe_unused]] gfx::ShaderRegistry& registry) {
    registerTypes<
        BackgroundProgram,
        BackgroundPatternProgram,
        RasterProgram,
        HeatmapProgram,
        HeatmapTextureProgram,
        HillshadeProgram,
        HillshadePrepareProgram,
        FillProgram,
        FillPatternProgram,
        FillOutlineProgram,
        FillOutlinePatternProgram,
        FillExtrusionProgram,
        FillExtrusionPatternProgram,
        CircleProgram,
        LineProgram,
        LineGradientProgram,
        LineSDFProgram,
        LinePatternProgram,
        SymbolIconProgram,
        SymbolSDFIconProgram,
        SymbolSDFTextProgram,
        SymbolTextAndIconProgram,
        CollisionBoxProgram,
        CollisionCircleProgram,
        DebugProgram,
        ClippingMaskProgram
    >(registry, programParameters);
}

} // namespace mbgl
