// Generated code, do not modify this file!
#pragma once
#include <mbgl/gfx/backend.hpp>

namespace mbgl {
namespace shaders {

enum class BuiltIn {
    None,
    Prelude,
    BackgroundProgram,
    BackgroundPatternProgram,
    CircleProgram,
    ClippingMaskProgram,
    CollisionBoxProgram,
    CollisionCircleProgram,
    DebugProgram,
    FillExtrusionPatternProgram,
    FillExtrusionProgram,
    FillOutlinePatternProgram,
    FillOutlineProgram,
    FillPatternProgram,
    FillProgram,
    HeatmapTextureProgram,
    HeatmapProgram,
    HillshadePrepareProgram,
    HillshadeProgram,
    LineGradientProgram,
    LinePatternProgram,
    LineSDFProgram,
    LineProgram,
    RasterProgram,
    SymbolIconProgram,
    SymbolSDFTextProgram,
    SymbolSDFIconProgram,
    SymbolTextAndIconProgram
};

template <BuiltIn T, gfx::Backend::Type> struct ShaderSource;

template <> struct ShaderSource<BuiltIn::None, gfx::Backend::Type::OpenGL> {
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

} // namespace shaders
} // namespace mbgl