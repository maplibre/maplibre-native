#include <mbgl/shaders/gl/shader_info.hpp>

#include <mbgl/shaders/ubo_max_count.hpp>

namespace mbgl {
namespace shaders {

UniformBlockInfo::UniformBlockInfo(std::string_view name_, std::size_t id_)
    : name(name_),
      id(id_),
      binding(id_) {}

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"BackgroundDrawableUBO", idBackgroundDrawableUBO},
    UniformBlockInfo{"BackgroundLayerUBO", idBackgroundLayerUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"BackgroundDrawableUBO", idBackgroundDrawableUBO},
        UniformBlockInfo{"BackgroundLayerUBO", idBackgroundLayerUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::CircleShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"CircleDrawableUBO", idCircleDrawableUBO},
    UniformBlockInfo{"CirclePaintParamsUBO", idCirclePaintParamsUBO},
    UniformBlockInfo{"CircleEvaluatedPropsUBO", idCircleEvaluatedPropsUBO},
    UniformBlockInfo{"CircleInterpolateUBO", idCircleInterpolateUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::CollisionBoxShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"CollisionBoxUBO", idCollisionUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"CollisionCircleUBO", idCollisionUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"DebugUBO", idDebugUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"FillDrawableUBO", idFillDrawableUBO},
    UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
    UniformBlockInfo{"FillInterpolateUBO", idFillInterpolateUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"FillOutlineDrawableUBO", idFillOutlineDrawableUBO},
        UniformBlockInfo{"FillOutlineEvaluatedPropsUBO", idFillOutlineEvaluatedPropsUBO},
        UniformBlockInfo{"FillOutlineInterpolateUBO", idFillOutlineInterpolateUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"FillPatternDrawableUBO", idFillPatternDrawableUBO},
        UniformBlockInfo{"FillPatternTilePropsUBO", idFillPatternTilePropsUBO},
        UniformBlockInfo{"FillPatternEvaluatedPropsUBO", idFillPatternEvaluatedPropsUBO},
        UniformBlockInfo{"FillPatternInterpolateUBO", idFillPatternInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillOutlinePatternDrawableUBO", idFillOutlinePatternDrawableUBO},
        UniformBlockInfo{"FillOutlinePatternTilePropsUBO", idFillOutlinePatternTilePropsUBO},
        UniformBlockInfo{"FillOutlinePatternEvaluatedPropsUBO", idFillOutlinePatternEvaluatedPropsUBO},
        UniformBlockInfo{"FillOutlinePatternInterpolateUBO", idFillOutlinePatternInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillExtrusionShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO},
        UniformBlockInfo{"FillExtrusionDrawablePropsUBO", idFillExtrusionDrawablePropsUBO},
        UniformBlockInfo{"FillExtrusionDrawableTilePropsUBO", idFillExtrusionDrawableTilePropsUBO},
        UniformBlockInfo{"FillExtrusionInterpolateUBO", idFillExtrusionInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO},
        UniformBlockInfo{"FillExtrusionDrawablePropsUBO", idFillExtrusionDrawablePropsUBO},
        UniformBlockInfo{"FillExtrusionDrawableTilePropsUBO", idFillExtrusionDrawableTilePropsUBO},
        UniformBlockInfo{"FillExtrusionInterpolateUBO", idFillExtrusionInterpolateUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"HeatmapDrawableUBO", idHeatmapDrawableUBO},
    UniformBlockInfo{"HeatmapEvaluatedPropsUBO", idHeatmapEvaluatedPropsUBO},
    UniformBlockInfo{"HeatmapInterpolateUBO", idHeatmapInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"HeatmapTextureDrawableUBO", idHeatmapTextureDrawableUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"HillshadePrepareDrawableUBO", idHillshadePrepareDrawableUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"HillshadeDrawableUBO", idHillshadeDrawableUBO},
    UniformBlockInfo{"HillshadeEvaluatedPropsUBO", idHillshadeEvaluatedPropsUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineGradientShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
        UniformBlockInfo{"LineGradientUBO", idLineGradientUBO},
        UniformBlockInfo{"LineGradientPropertiesUBO", idLineGradientPropertiesUBO},
        UniformBlockInfo{"LineGradientInterpolationUBO", idLineGradientInterpolationUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LinePatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
        UniformBlockInfo{"LinePatternUBO", idLinePatternUBO},
        UniformBlockInfo{"LinePatternPropertiesUBO", idLinePatternPropertiesUBO},
        UniformBlockInfo{"LinePatternInterpolationUBO", idLinePatternInterpolationUBO},
        UniformBlockInfo{"LinePatternTilePropertiesUBO", idLinePatternTilePropertiesUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineSDFShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
    UniformBlockInfo{"LineSDFUBO", idLineSDFUBO},
    UniformBlockInfo{"LineSDFPropertiesUBO", idLineSDFPropertiesUBO},
    UniformBlockInfo{"LineSDFInterpolationUBO", idLineSDFInterpolationUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
    UniformBlockInfo{"LineUBO", idLineUBO},
    UniformBlockInfo{"LinePropertiesUBO", idLinePropertiesUBO},
    UniformBlockInfo{"LineInterpolationUBO", idLineInterpolationUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineBasicShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineBasicUBO", idLineBasicUBO},
    UniformBlockInfo{"LineBasicPropertiesUBO", idLineBasicPropertiesUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"RasterDrawableUBO", idRasterDrawableUBO},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
    UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO},
    UniformBlockInfo{"SymbolDrawablePaintUBO", idSymbolDrawablePaintUBO},
    UniformBlockInfo{"SymbolDrawableTilePropsUBO", idSymbolDrawableTilePropsUBO},
    UniformBlockInfo{"SymbolDrawableInterpolateUBO", idSymbolDrawableInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
        UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO},
        UniformBlockInfo{"SymbolDrawablePaintUBO", idSymbolDrawablePaintUBO},
        UniformBlockInfo{"SymbolDrawableTilePropsUBO", idSymbolDrawableTilePropsUBO},
        UniformBlockInfo{"SymbolDrawableInterpolateUBO", idSymbolDrawableInterpolateUBO},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
        UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO},
        UniformBlockInfo{"SymbolDrawablePaintUBO", idSymbolDrawablePaintUBO},
        UniformBlockInfo{"SymbolDrawableTilePropsUBO", idSymbolDrawableTilePropsUBO},
        UniformBlockInfo{"SymbolDrawableInterpolateUBO", idSymbolDrawableInterpolateUBO},
};

} // namespace shaders
} // namespace mbgl
