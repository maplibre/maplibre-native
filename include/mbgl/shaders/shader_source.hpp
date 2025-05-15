// Generated code, do not modify this file!
#pragma once
#include <mbgl/gfx/backend.hpp>

namespace mbgl {
namespace shaders {

/// @brief This enum is used with the ShaderSource template to select
/// source code for the desired program and graphics back-end.
enum class BuiltIn {
    None,
    Prelude,
    ClippingMaskProgram,
    BackgroundShader,
    BackgroundPatternShader,
    CircleShader,
    CollisionBoxShader,
    CollisionCircleShader,
    CustomGeometryShader,
    CustomSymbolIconShader,
    DebugShader,
    FillShader,
    FillOutlineShader,
    FillPatternShader,
    FillOutlinePatternShader,
    FillOutlineTriangulatedShader,
    FillExtrusionShader,
    FillExtrusionPatternShader,
    HeatmapShader,
    HeatmapTextureShader,
    HillshadePrepareShader,
    HillshadeShader,
    LineShader,
    LineGradientShader,
    LinePatternShader,
    LocationIndicatorShader,
    LocationIndicatorTexturedShader,
    LineSDFShader,
    RasterShader,
    SymbolIconShader,
    SymbolSDFIconShader,
    SymbolTextAndIconShader,
    WideVectorShader
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
    static constexpr const char* name = "";
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

} // namespace shaders
} // namespace mbgl
