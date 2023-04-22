// Generated code, do not modify this file!
// Generated on 2023-04-05T16:25:15.886Z by mwilsnd using shaders/generate_shader_code.js

#pragma once
#include <mbgl/gfx/backend.hpp>

namespace mbgl {
namespace shaders {

/// @brief This enum is used with the ShaderSource template to select
/// source code for the desired program and graphics back-end.
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

/// @brief Select shader source based on a program type and a desired
/// graphics API.
/// @tparam T One of the built-in shader types available in the BuiltIn enum
/// @tparam The desired graphics API to request shader code for. One of
/// gfx::Backend::Type enums.
template <BuiltIn T, gfx::Backend::Type>
struct ShaderSource;

/// @brief A specialization of the ShaderSource template for no shader code.
template <>
struct ShaderSource<BuiltIn::None, gfx::Backend::Type::OpenGL> {
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

} // namespace shaders
} // namespace mbgl
