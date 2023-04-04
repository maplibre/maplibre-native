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
    : debug(context_, programParameters_),
      clippingMask(context_, programParameters_),
      context(context_),
      programParameters(programParameters_) {}

Programs::~Programs() = default;

template<typename ... T>
void registerTypes(gfx::ShaderRegistry& registry,
    gfx::Context& context_, const ProgramParameters& programParameters_)
{
    ( [](bool expr) {
        if (!expr) {
            throw std::runtime_error(
                "Failed to register " +
                std::string(T::Name) +
                " with shader registry!");
        }
    }( registry.registerShader(
        std::make_shared<T>(context_, programParameters_))), ... );
}

void Programs::registerWith([[maybe_unused]] gfx::ShaderRegistry& registry) {
    try {
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
        >(registry, context, programParameters);
    } catch (const std::runtime_error& e) {
        Log::Error(Event::Shader, e.what());
        assert(0 && "Programs::registerWith failed");
        std::rethrow_exception(std::current_exception());
    }
}

BackgroundLayerPrograms& Programs::getBackgroundLayerPrograms() noexcept {
    if (!backgroundPrograms) {
        backgroundPrograms = std::make_unique<BackgroundLayerPrograms>(context, programParameters);
    }
    return static_cast<BackgroundLayerPrograms&>(*backgroundPrograms);   
}

RasterLayerPrograms& Programs::getRasterLayerPrograms() noexcept {
    if (!rasterPrograms) {
        rasterPrograms = std::make_unique<RasterLayerPrograms>(context, programParameters);
    }
    return static_cast<RasterLayerPrograms&>(*rasterPrograms);   
}

HeatmapLayerPrograms& Programs::getHeatmapLayerPrograms() noexcept {
    if (!heatmapPrograms) {
        heatmapPrograms = std::make_unique<HeatmapLayerPrograms>(context, programParameters);
    }
    return static_cast<HeatmapLayerPrograms&>(*heatmapPrograms);   
}

HillshadeLayerPrograms& Programs::getHillshadeLayerPrograms() noexcept {
    if (!hillshadePrograms) {
        hillshadePrograms = std::make_unique<HillshadeLayerPrograms>(context, programParameters);
    }
    return static_cast<HillshadeLayerPrograms&>(*hillshadePrograms);   
}

FillLayerPrograms& Programs::getFillLayerPrograms() noexcept {
    if (!fillPrograms) {
        fillPrograms = std::make_unique<FillLayerPrograms>(context, programParameters);
    }
    return static_cast<FillLayerPrograms&>(*fillPrograms);   
}

FillExtrusionLayerPrograms& Programs::getFillExtrusionLayerPrograms() noexcept {
    if (!fillExtrusionPrograms) {
        fillExtrusionPrograms = std::make_unique<FillExtrusionLayerPrograms>(context, programParameters);
    }
    return static_cast<FillExtrusionLayerPrograms&>(*fillExtrusionPrograms);   
}

CircleLayerPrograms& Programs::getCircleLayerPrograms() noexcept {
    if (!circlePrograms) {
        circlePrograms = std::make_unique<CircleLayerPrograms>(context, programParameters);
    }
    return static_cast<CircleLayerPrograms&>(*circlePrograms);   
}

LineLayerPrograms& Programs::getLineLayerPrograms() noexcept {
    if (!linePrograms) {
        linePrograms = std::make_unique<LineLayerPrograms>(context, programParameters);
    }
    return static_cast<LineLayerPrograms&>(*linePrograms);   
}

SymbolLayerPrograms& Programs::getSymbolLayerPrograms() noexcept {
    if (!symbolPrograms) {
        symbolPrograms = std::make_unique<SymbolLayerPrograms>(context, programParameters);
    }
    return static_cast<SymbolLayerPrograms&>(*symbolPrograms);   
}

} // namespace mbgl
