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

namespace mbgl {

Programs::Programs(const ProgramParameters& programParameters_)
    : programParameters(programParameters_) {}

Programs::~Programs() = default;

namespace {
/// @brief Register a list of types with a shader registry instance
/// @tparam ...T Type list parameter pack
/// @param registry A shader registry instance
/// programParameters_ ProgramParameters used to initialize each instance
template <typename... T>
void registerTypes(gfx::ShaderRegistry& registry, const ProgramParameters& programParameters_) {
    /// The following fold expression will create a shared_ptr for every type
    /// in the parameter pack and register it with the shader registry.

    /// Registration calls are wrapped in a lambda that throws on registration
    /// failure, we shouldn't expect registration to faill unless the shader
    /// registry instance provided already has conflicting programs present.
    (
        [](bool expr) {
            if (!expr) {
                throw std::runtime_error("Failed to register " + std::string(T::Name) + " with shader registry!");
            }
        }(registry.getLegacyGroup().registerShader(std::make_shared<T>(programParameters_))),
        ...);
}
} // namespace

void Programs::registerWith(gfx::ShaderRegistry& registry) {
#if MLN_LEGACY_RENDERER
    /// The following types will be registered
    registerTypes<BackgroundProgram,
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
                  ClippingMaskProgram>(registry, programParameters);
#else
    /// The following types will be registered
    registerTypes<DebugProgram, ClippingMaskProgram>(registry, programParameters);
#endif
}

} // namespace mbgl
