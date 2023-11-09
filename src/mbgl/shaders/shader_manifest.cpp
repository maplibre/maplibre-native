// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/shader_manifest.hpp>
#include <type_traits>
#include <cassert>
#include <stdexcept>
#include <string>

namespace mbgl {
namespace shaders {

constexpr const char* programNames[] = {"None", // shaders::BuiltIn::None = 0,
                                        "BackgroundShader",
                                        "BackgroundPatternShader",
                                        "CircleShader",
                                        "CollisionBoxShader",
                                        "CollisionCircleShader",
                                        "DebugShader",
                                        "FillShader",
                                        "FillOutlineShader",
                                        "LineGradientShader",
                                        "LinePatternShader",
                                        "LineSDFShader",
                                        "LineShader",
                                        "FillPatternShader",
                                        "FillOutlinePatternShader",
                                        "FillExtrusionShader",
                                        "FillExtrusionPatternShader",
                                        "HeatmapShader",
                                        "HeatmapTextureShader",
                                        "HillshadePrepareShader",
                                        "HillshadeShader",
                                        "RasterShader",
                                        "SymbolIconShader",
                                        "SymbolSDFIconShader",
                                        "SymbolTextAndIconShader",
                                        "Prelude",
                                        "BackgroundProgram",
                                        "BackgroundPatternProgram",
                                        "CircleProgram",
                                        "ClippingMaskProgram",
                                        "CollisionBoxProgram",
                                        "CollisionCircleProgram",
                                        "DebugProgram",
                                        "FillExtrusionPatternProgram",
                                        "FillExtrusionProgram",
                                        "FillOutlinePatternProgram",
                                        "FillOutlineProgram",
                                        "FillPatternProgram",
                                        "FillProgram",
                                        "HeatmapTextureProgram",
                                        "HeatmapProgram",
                                        "HillshadePrepareProgram",
                                        "HillshadeProgram",
                                        "LineGradientProgram",
                                        "LinePatternProgram",
                                        "LineSDFProgram",
                                        "LineProgram",
                                        "RasterProgram",
                                        "SymbolIconProgram",
                                        "SymbolSDFTextProgram",
                                        "SymbolSDFIconProgram",
                                        "SymbolTextAndIconProgram"};

std::string getProgramName(shaders::BuiltIn programID) {
    assert(static_cast<int64_t>(programID) > static_cast<int64_t>(shaders::BuiltIn::None));
    assert(static_cast<int64_t>(programID) <= 51);
    return programNames[static_cast<size_t>(programID)];
}

#if MLN_RENDER_BACKEND_OPENGL
template <BuiltIn ID>
using GLProgram = mbgl::shaders::ShaderSource<ID, gfx::Backend::Type::OpenGL>;

template <>
std::pair<std::string, std::string> getShaderSource<gfx::Backend::Type::OpenGL>(shaders::BuiltIn programID) {
    switch (programID) {
        case BuiltIn::BackgroundShader: {
            return std::make_pair(GLProgram<BuiltIn::BackgroundShader>::vertex(),
                                  GLProgram<BuiltIn::BackgroundShader>::fragment());
        }
        case BuiltIn::BackgroundPatternShader: {
            return std::make_pair(GLProgram<BuiltIn::BackgroundPatternShader>::vertex(),
                                  GLProgram<BuiltIn::BackgroundPatternShader>::fragment());
        }
        case BuiltIn::CircleShader: {
            return std::make_pair(GLProgram<BuiltIn::CircleShader>::vertex(),
                                  GLProgram<BuiltIn::CircleShader>::fragment());
        }
        case BuiltIn::CollisionBoxShader: {
            return std::make_pair(GLProgram<BuiltIn::CollisionBoxShader>::vertex(),
                                  GLProgram<BuiltIn::CollisionBoxShader>::fragment());
        }
        case BuiltIn::CollisionCircleShader: {
            return std::make_pair(GLProgram<BuiltIn::CollisionCircleShader>::vertex(),
                                  GLProgram<BuiltIn::CollisionCircleShader>::fragment());
        }
        case BuiltIn::DebugShader: {
            return std::make_pair(GLProgram<BuiltIn::DebugShader>::vertex(),
                                  GLProgram<BuiltIn::DebugShader>::fragment());
        }
        case BuiltIn::FillShader: {
            return std::make_pair(GLProgram<BuiltIn::FillShader>::vertex(), GLProgram<BuiltIn::FillShader>::fragment());
        }
        case BuiltIn::FillOutlineShader: {
            return std::make_pair(GLProgram<BuiltIn::FillOutlineShader>::vertex(),
                                  GLProgram<BuiltIn::FillOutlineShader>::fragment());
        }
        case BuiltIn::LineGradientShader: {
            return std::make_pair(GLProgram<BuiltIn::LineGradientShader>::vertex(),
                                  GLProgram<BuiltIn::LineGradientShader>::fragment());
        }
        case BuiltIn::LinePatternShader: {
            return std::make_pair(GLProgram<BuiltIn::LinePatternShader>::vertex(),
                                  GLProgram<BuiltIn::LinePatternShader>::fragment());
        }
        case BuiltIn::LineSDFShader: {
            return std::make_pair(GLProgram<BuiltIn::LineSDFShader>::vertex(),
                                  GLProgram<BuiltIn::LineSDFShader>::fragment());
        }
        case BuiltIn::LineShader: {
            return std::make_pair(GLProgram<BuiltIn::LineShader>::vertex(), GLProgram<BuiltIn::LineShader>::fragment());
        }
        case BuiltIn::FillPatternShader: {
            return std::make_pair(GLProgram<BuiltIn::FillPatternShader>::vertex(),
                                  GLProgram<BuiltIn::FillPatternShader>::fragment());
        }
        case BuiltIn::FillOutlinePatternShader: {
            return std::make_pair(GLProgram<BuiltIn::FillOutlinePatternShader>::vertex(),
                                  GLProgram<BuiltIn::FillOutlinePatternShader>::fragment());
        }
        case BuiltIn::FillExtrusionShader: {
            return std::make_pair(GLProgram<BuiltIn::FillExtrusionShader>::vertex(),
                                  GLProgram<BuiltIn::FillExtrusionShader>::fragment());
        }
        case BuiltIn::FillExtrusionPatternShader: {
            return std::make_pair(GLProgram<BuiltIn::FillExtrusionPatternShader>::vertex(),
                                  GLProgram<BuiltIn::FillExtrusionPatternShader>::fragment());
        }
        case BuiltIn::HeatmapShader: {
            return std::make_pair(GLProgram<BuiltIn::HeatmapShader>::vertex(),
                                  GLProgram<BuiltIn::HeatmapShader>::fragment());
        }
        case BuiltIn::HeatmapTextureShader: {
            return std::make_pair(GLProgram<BuiltIn::HeatmapTextureShader>::vertex(),
                                  GLProgram<BuiltIn::HeatmapTextureShader>::fragment());
        }
        case BuiltIn::HillshadePrepareShader: {
            return std::make_pair(GLProgram<BuiltIn::HillshadePrepareShader>::vertex(),
                                  GLProgram<BuiltIn::HillshadePrepareShader>::fragment());
        }
        case BuiltIn::HillshadeShader: {
            return std::make_pair(GLProgram<BuiltIn::HillshadeShader>::vertex(),
                                  GLProgram<BuiltIn::HillshadeShader>::fragment());
        }
        case BuiltIn::RasterShader: {
            return std::make_pair(GLProgram<BuiltIn::RasterShader>::vertex(),
                                  GLProgram<BuiltIn::RasterShader>::fragment());
        }
        case BuiltIn::SymbolIconShader: {
            return std::make_pair(GLProgram<BuiltIn::SymbolIconShader>::vertex(),
                                  GLProgram<BuiltIn::SymbolIconShader>::fragment());
        }
        case BuiltIn::SymbolSDFIconShader: {
            return std::make_pair(GLProgram<BuiltIn::SymbolSDFIconShader>::vertex(),
                                  GLProgram<BuiltIn::SymbolSDFIconShader>::fragment());
        }
        case BuiltIn::SymbolTextAndIconShader: {
            return std::make_pair(GLProgram<BuiltIn::SymbolTextAndIconShader>::vertex(),
                                  GLProgram<BuiltIn::SymbolTextAndIconShader>::fragment());
        }
        case BuiltIn::Prelude: {
            return std::make_pair(GLProgram<BuiltIn::Prelude>::vertex(), GLProgram<BuiltIn::Prelude>::fragment());
        }
        case BuiltIn::BackgroundProgram: {
            return std::make_pair(GLProgram<BuiltIn::BackgroundProgram>::vertex(),
                                  GLProgram<BuiltIn::BackgroundProgram>::fragment());
        }
        case BuiltIn::BackgroundPatternProgram: {
            return std::make_pair(GLProgram<BuiltIn::BackgroundPatternProgram>::vertex(),
                                  GLProgram<BuiltIn::BackgroundPatternProgram>::fragment());
        }
        case BuiltIn::CircleProgram: {
            return std::make_pair(GLProgram<BuiltIn::CircleProgram>::vertex(),
                                  GLProgram<BuiltIn::CircleProgram>::fragment());
        }
        case BuiltIn::ClippingMaskProgram: {
            return std::make_pair(GLProgram<BuiltIn::ClippingMaskProgram>::vertex(),
                                  GLProgram<BuiltIn::ClippingMaskProgram>::fragment());
        }
        case BuiltIn::CollisionBoxProgram: {
            return std::make_pair(GLProgram<BuiltIn::CollisionBoxProgram>::vertex(),
                                  GLProgram<BuiltIn::CollisionBoxProgram>::fragment());
        }
        case BuiltIn::CollisionCircleProgram: {
            return std::make_pair(GLProgram<BuiltIn::CollisionCircleProgram>::vertex(),
                                  GLProgram<BuiltIn::CollisionCircleProgram>::fragment());
        }
        case BuiltIn::DebugProgram: {
            return std::make_pair(GLProgram<BuiltIn::DebugProgram>::vertex(),
                                  GLProgram<BuiltIn::DebugProgram>::fragment());
        }
        case BuiltIn::FillExtrusionPatternProgram: {
            return std::make_pair(GLProgram<BuiltIn::FillExtrusionPatternProgram>::vertex(),
                                  GLProgram<BuiltIn::FillExtrusionPatternProgram>::fragment());
        }
        case BuiltIn::FillExtrusionProgram: {
            return std::make_pair(GLProgram<BuiltIn::FillExtrusionProgram>::vertex(),
                                  GLProgram<BuiltIn::FillExtrusionProgram>::fragment());
        }
        case BuiltIn::FillOutlinePatternProgram: {
            return std::make_pair(GLProgram<BuiltIn::FillOutlinePatternProgram>::vertex(),
                                  GLProgram<BuiltIn::FillOutlinePatternProgram>::fragment());
        }
        case BuiltIn::FillOutlineProgram: {
            return std::make_pair(GLProgram<BuiltIn::FillOutlineProgram>::vertex(),
                                  GLProgram<BuiltIn::FillOutlineProgram>::fragment());
        }
        case BuiltIn::FillPatternProgram: {
            return std::make_pair(GLProgram<BuiltIn::FillPatternProgram>::vertex(),
                                  GLProgram<BuiltIn::FillPatternProgram>::fragment());
        }
        case BuiltIn::FillProgram: {
            return std::make_pair(GLProgram<BuiltIn::FillProgram>::vertex(),
                                  GLProgram<BuiltIn::FillProgram>::fragment());
        }
        case BuiltIn::HeatmapTextureProgram: {
            return std::make_pair(GLProgram<BuiltIn::HeatmapTextureProgram>::vertex(),
                                  GLProgram<BuiltIn::HeatmapTextureProgram>::fragment());
        }
        case BuiltIn::HeatmapProgram: {
            return std::make_pair(GLProgram<BuiltIn::HeatmapProgram>::vertex(),
                                  GLProgram<BuiltIn::HeatmapProgram>::fragment());
        }
        case BuiltIn::HillshadePrepareProgram: {
            return std::make_pair(GLProgram<BuiltIn::HillshadePrepareProgram>::vertex(),
                                  GLProgram<BuiltIn::HillshadePrepareProgram>::fragment());
        }
        case BuiltIn::HillshadeProgram: {
            return std::make_pair(GLProgram<BuiltIn::HillshadeProgram>::vertex(),
                                  GLProgram<BuiltIn::HillshadeProgram>::fragment());
        }
        case BuiltIn::LineGradientProgram: {
            return std::make_pair(GLProgram<BuiltIn::LineGradientProgram>::vertex(),
                                  GLProgram<BuiltIn::LineGradientProgram>::fragment());
        }
        case BuiltIn::LinePatternProgram: {
            return std::make_pair(GLProgram<BuiltIn::LinePatternProgram>::vertex(),
                                  GLProgram<BuiltIn::LinePatternProgram>::fragment());
        }
        case BuiltIn::LineSDFProgram: {
            return std::make_pair(GLProgram<BuiltIn::LineSDFProgram>::vertex(),
                                  GLProgram<BuiltIn::LineSDFProgram>::fragment());
        }
        case BuiltIn::LineProgram: {
            return std::make_pair(GLProgram<BuiltIn::LineProgram>::vertex(),
                                  GLProgram<BuiltIn::LineProgram>::fragment());
        }
        case BuiltIn::RasterProgram: {
            return std::make_pair(GLProgram<BuiltIn::RasterProgram>::vertex(),
                                  GLProgram<BuiltIn::RasterProgram>::fragment());
        }
        case BuiltIn::SymbolIconProgram: {
            return std::make_pair(GLProgram<BuiltIn::SymbolIconProgram>::vertex(),
                                  GLProgram<BuiltIn::SymbolIconProgram>::fragment());
        }
        case BuiltIn::SymbolSDFTextProgram: {
            return std::make_pair(GLProgram<BuiltIn::SymbolSDFTextProgram>::vertex(),
                                  GLProgram<BuiltIn::SymbolSDFTextProgram>::fragment());
        }
        case BuiltIn::SymbolSDFIconProgram: {
            return std::make_pair(GLProgram<BuiltIn::SymbolSDFIconProgram>::vertex(),
                                  GLProgram<BuiltIn::SymbolSDFIconProgram>::fragment());
        }
        case BuiltIn::SymbolTextAndIconProgram: {
            return std::make_pair(GLProgram<BuiltIn::SymbolTextAndIconProgram>::vertex(),
                                  GLProgram<BuiltIn::SymbolTextAndIconProgram>::fragment());
        }
        default: {
            return {"", ""};
        }
    }
}
#endif

#if MLN_RENDER_BACKEND_METAL
template <BuiltIn ID>
using MtlProgram = mbgl::shaders::ShaderSource<ID, gfx::Backend::Type::Metal>;

template <>
std::pair<std::string, std::string> getShaderSource<gfx::Backend::Type::Metal>(shaders::BuiltIn programID) {
    switch (programID) {
        case BuiltIn::BackgroundShader: {
            return std::make_pair(MtlProgram<BuiltIn::BackgroundShader>::source(), "");
        }
        case BuiltIn::BackgroundPatternShader: {
            return std::make_pair(MtlProgram<BuiltIn::BackgroundPatternShader>::source(), "");
        }
        case BuiltIn::CircleShader: {
            return std::make_pair(MtlProgram<BuiltIn::CircleShader>::source(), "");
        }
        case BuiltIn::CollisionBoxShader: {
            return std::make_pair(MtlProgram<BuiltIn::CollisionBoxShader>::source(), "");
        }
        case BuiltIn::CollisionCircleShader: {
            return std::make_pair(MtlProgram<BuiltIn::CollisionCircleShader>::source(), "");
        }
        case BuiltIn::FillShader: {
            return std::make_pair(MtlProgram<BuiltIn::FillShader>::source(), "");
        }
        case BuiltIn::FillOutlineShader: {
            return std::make_pair(MtlProgram<BuiltIn::FillOutlineShader>::source(), "");
        }
        case BuiltIn::LineGradientShader: {
            return std::make_pair(MtlProgram<BuiltIn::LineGradientShader>::source(), "");
        }
        case BuiltIn::LinePatternShader: {
            return std::make_pair(MtlProgram<BuiltIn::LinePatternShader>::source(), "");
        }
        case BuiltIn::LineSDFShader: {
            return std::make_pair(MtlProgram<BuiltIn::LineSDFShader>::source(), "");
        }
        case BuiltIn::LineShader: {
            return std::make_pair(MtlProgram<BuiltIn::LineShader>::source(), "");
        }
        case BuiltIn::FillPatternShader: {
            return std::make_pair(MtlProgram<BuiltIn::FillPatternShader>::source(), "");
        }
        case BuiltIn::FillOutlinePatternShader: {
            return std::make_pair(MtlProgram<BuiltIn::FillOutlinePatternShader>::source(), "");
        }
        case BuiltIn::FillExtrusionShader: {
            return std::make_pair(MtlProgram<BuiltIn::FillExtrusionShader>::source(), "");
        }
        case BuiltIn::FillExtrusionPatternShader: {
            return std::make_pair(MtlProgram<BuiltIn::FillExtrusionPatternShader>::source(), "");
        }
        case BuiltIn::HeatmapShader: {
            return std::make_pair(MtlProgram<BuiltIn::HeatmapShader>::source(), "");
        }
        case BuiltIn::HeatmapTextureShader: {
            return std::make_pair(MtlProgram<BuiltIn::HeatmapTextureShader>::source(), "");
        }
        case BuiltIn::HillshadePrepareShader: {
            return std::make_pair(MtlProgram<BuiltIn::HillshadePrepareShader>::source(), "");
        }
        case BuiltIn::HillshadeShader: {
            return std::make_pair(MtlProgram<BuiltIn::HillshadeShader>::source(), "");
        }
        case BuiltIn::RasterShader: {
            return std::make_pair(MtlProgram<BuiltIn::RasterShader>::source(), "");
        }
        case BuiltIn::SymbolIconShader: {
            return std::make_pair(MtlProgram<BuiltIn::SymbolIconShader>::source(), "");
        }
        case BuiltIn::SymbolSDFIconShader: {
            return std::make_pair(MtlProgram<BuiltIn::SymbolSDFIconShader>::source(), "");
        }
        case BuiltIn::SymbolTextAndIconShader: {
            return std::make_pair(MtlProgram<BuiltIn::SymbolTextAndIconShader>::source(), "");
        }
        case BuiltIn::Prelude: {
            return std::make_pair(MtlProgram<BuiltIn::Prelude>::source(), "");
        }
        case BuiltIn::ClippingMaskProgram: {
            return std::make_pair(MtlProgram<BuiltIn::ClippingMaskProgram>::source(), "");
        }
        default: {
            throw std::runtime_error("No source found for program!");
        }
    }
}

template <>
const ReflectionData& getReflectionData<gfx::Backend::Type::Metal>(shaders::BuiltIn programID) {
    switch (programID) {
        case BuiltIn::BackgroundShader: {
            return MtlProgram<BuiltIn::BackgroundShader>::reflectionData;
        }
        case BuiltIn::BackgroundPatternShader: {
            return MtlProgram<BuiltIn::BackgroundPatternShader>::reflectionData;
        }
        case BuiltIn::CircleShader: {
            return MtlProgram<BuiltIn::CircleShader>::reflectionData;
        }
        case BuiltIn::CollisionBoxShader: {
            return MtlProgram<BuiltIn::CollisionBoxShader>::reflectionData;
        }
        case BuiltIn::CollisionCircleShader: {
            return MtlProgram<BuiltIn::CollisionCircleShader>::reflectionData;
        }
        case BuiltIn::FillShader: {
            return MtlProgram<BuiltIn::FillShader>::reflectionData;
        }
        case BuiltIn::FillOutlineShader: {
            return MtlProgram<BuiltIn::FillOutlineShader>::reflectionData;
        }
        case BuiltIn::LineGradientShader: {
            return MtlProgram<BuiltIn::LineGradientShader>::reflectionData;
        }
        case BuiltIn::LinePatternShader: {
            return MtlProgram<BuiltIn::LinePatternShader>::reflectionData;
        }
        case BuiltIn::LineSDFShader: {
            return MtlProgram<BuiltIn::LineSDFShader>::reflectionData;
        }
        case BuiltIn::LineShader: {
            return MtlProgram<BuiltIn::LineShader>::reflectionData;
        }
        case BuiltIn::FillPatternShader: {
            return MtlProgram<BuiltIn::FillPatternShader>::reflectionData;
        }
        case BuiltIn::FillOutlinePatternShader: {
            return MtlProgram<BuiltIn::FillOutlinePatternShader>::reflectionData;
        }
        case BuiltIn::FillExtrusionShader: {
            return MtlProgram<BuiltIn::FillExtrusionShader>::reflectionData;
        }
        case BuiltIn::FillExtrusionPatternShader: {
            return MtlProgram<BuiltIn::FillExtrusionPatternShader>::reflectionData;
        }
        case BuiltIn::HeatmapShader: {
            return MtlProgram<BuiltIn::HeatmapShader>::reflectionData;
        }
        case BuiltIn::HeatmapTextureShader: {
            return MtlProgram<BuiltIn::HeatmapTextureShader>::reflectionData;
        }
        case BuiltIn::HillshadePrepareShader: {
            return MtlProgram<BuiltIn::HillshadePrepareShader>::reflectionData;
        }
        case BuiltIn::HillshadeShader: {
            return MtlProgram<BuiltIn::HillshadeShader>::reflectionData;
        }
        case BuiltIn::RasterShader: {
            return MtlProgram<BuiltIn::RasterShader>::reflectionData;
        }
        case BuiltIn::SymbolIconShader: {
            return MtlProgram<BuiltIn::SymbolIconShader>::reflectionData;
        }
        case BuiltIn::SymbolSDFIconShader: {
            return MtlProgram<BuiltIn::SymbolSDFIconShader>::reflectionData;
        }
        case BuiltIn::SymbolTextAndIconShader: {
            return MtlProgram<BuiltIn::SymbolTextAndIconShader>::reflectionData;
        }
        case BuiltIn::ClippingMaskProgram: {
            return MtlProgram<BuiltIn::ClippingMaskProgram>::reflectionData;
        }
        default: {
            throw std::runtime_error("No reflection data found for program!");
        }
    }
}
#endif

} // namespace shaders
} // namespace mbgl

// NOLINTEND
