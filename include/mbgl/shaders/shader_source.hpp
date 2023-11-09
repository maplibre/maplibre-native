// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/gfx/backend.hpp>
#include <string>
#include <utility>

#define MLN_COMPRESSED_SHADERS 0
#define MLN_AMALGAMATED_SHADERS 0
namespace mbgl {
namespace shaders {

/// @brief This enum is used with the ShaderSource template to select
/// source code for the desired program and graphics back-end.
enum class BuiltIn : uint8_t {
    None = 0,
    BackgroundShader,
    BackgroundPatternShader,
    CircleShader,
    CollisionBoxShader,
    CollisionCircleShader,
    DebugShader,
    FillShader,
    FillOutlineShader,
    LineGradientShader,
    LinePatternShader,
    LineSDFShader,
    LineShader,
    FillPatternShader,
    FillOutlinePatternShader,
    FillExtrusionShader,
    FillExtrusionPatternShader,
    HeatmapShader,
    HeatmapTextureShader,
    HillshadePrepareShader,
    HillshadeShader,
    RasterShader,
    SymbolIconShader,
    SymbolSDFIconShader,
    SymbolTextAndIconShader,
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
    static constexpr const char* name = "";
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

template <>
struct ShaderSource<BuiltIn::None, gfx::Backend::Type::Metal> {
    static constexpr const char* name = "";
    static constexpr const char* vertexMainFunction = "";
    static constexpr const char* fragmentMainFunction = "";
    static constexpr const char* source = "";
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
