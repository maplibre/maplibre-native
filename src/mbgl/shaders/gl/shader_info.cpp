#include <mbgl/shaders/gl/shader_info.hpp>

#include <mbgl/shaders/ubo_max_count.hpp>

namespace mbgl {
namespace shaders {

UniformBlockInfo::UniformBlockInfo(std::string_view name_, std::size_t id_, std::size_t binding_)
    : name(name_),
      id(id_),
      binding(binding_) {}

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"BackgroundDrawableUBO", idBackgroundDrawableUBO, 0},
    UniformBlockInfo{"BackgroundLayerUBO", idBackgroundLayerUBO, 1},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"BackgroundDrawableUBO", idBackgroundDrawableUBO, 2},
        UniformBlockInfo{"BackgroundLayerUBO", idBackgroundLayerUBO, 3},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::CircleShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"CircleDrawableUBO", idCircleDrawableUBO, 0},
    UniformBlockInfo{"CirclePaintParamsUBO", idCirclePaintParamsUBO, 1},
    UniformBlockInfo{"CircleEvaluatedPropsUBO", idCircleEvaluatedPropsUBO, 2},
    UniformBlockInfo{"CircleInterpolateUBO", idCircleInterpolateUBO, 3},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::CollisionBoxShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"CollisionBoxUBO", idCollisionUBO, 19},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"CollisionCircleUBO", idCollisionUBO, 20},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"DebugUBO", idDebugUBO, 21},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"FillDrawableUBO", idFillDrawableUBO, 0},
    UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO, 1},
    UniformBlockInfo{"FillInterpolateUBO", idFillInterpolateUBO, 2},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"FillOutlineDrawableUBO", idFillOutlineDrawableUBO, 3},
        UniformBlockInfo{"FillOutlineEvaluatedPropsUBO", idFillOutlineEvaluatedPropsUBO, 4},
        UniformBlockInfo{"FillOutlineInterpolateUBO", idFillOutlineInterpolateUBO, 5},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"FillPatternDrawableUBO", idFillPatternDrawableUBO, 6},
        UniformBlockInfo{"FillPatternTilePropsUBO", idFillPatternTilePropsUBO, 7},
        UniformBlockInfo{"FillPatternEvaluatedPropsUBO", idFillPatternEvaluatedPropsUBO, 8},
        UniformBlockInfo{"FillPatternInterpolateUBO", idFillPatternInterpolateUBO, 9},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillOutlinePatternDrawableUBO", idFillOutlinePatternDrawableUBO, 10},
        UniformBlockInfo{"FillOutlinePatternTilePropsUBO", idFillOutlinePatternTilePropsUBO, 11},
        UniformBlockInfo{"FillOutlinePatternEvaluatedPropsUBO", idFillOutlinePatternEvaluatedPropsUBO, 12},
        UniformBlockInfo{"FillOutlinePatternInterpolateUBO", idFillOutlinePatternInterpolateUBO, 13},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillExtrusionShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO, 0},
        UniformBlockInfo{"FillExtrusionDrawablePropsUBO", idFillExtrusionDrawablePropsUBO, 1},
        UniformBlockInfo{"FillExtrusionDrawableTilePropsUBO", idFillExtrusionDrawableTilePropsUBO, 2},
        UniformBlockInfo{"FillExtrusionInterpolateUBO", idFillExtrusionInterpolateUBO, 3},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO, 4},
        UniformBlockInfo{"FillExtrusionDrawablePropsUBO", idFillExtrusionDrawablePropsUBO, 5},
        UniformBlockInfo{"FillExtrusionDrawableTilePropsUBO", idFillExtrusionDrawableTilePropsUBO, 6},
        UniformBlockInfo{"FillExtrusionInterpolateUBO", idFillExtrusionInterpolateUBO, 7},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"HeatmapDrawableUBO", idHeatmapDrawableUBO, 0},
    UniformBlockInfo{"HeatmapEvaluatedPropsUBO", idHeatmapEvaluatedPropsUBO, 1},
    UniformBlockInfo{"HeatmapInterpolateUBO", idHeatmapInterpolateUBO, 2},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"HeatmapTextureDrawableUBO", idHeatmapTextureDrawableUBO, 3},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"HillshadePrepareDrawableUBO", idHillshadePrepareDrawableUBO, 0},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"HillshadeDrawableUBO", idHillshadeDrawableUBO, 1},
    UniformBlockInfo{"HillshadeEvaluatedPropsUBO", idHillshadeEvaluatedPropsUBO, 2},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineGradientShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO, 0},
        UniformBlockInfo{"LineGradientUBO", idLineGradientUBO, 1},
        UniformBlockInfo{"LineGradientPropertiesUBO", idLineGradientPropertiesUBO, 2},
        UniformBlockInfo{"LineGradientInterpolationUBO", idLineGradientInterpolationUBO, 3},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LinePatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO, 4},
        UniformBlockInfo{"LinePatternUBO", idLinePatternUBO, 5},
        UniformBlockInfo{"LinePatternPropertiesUBO", idLinePatternPropertiesUBO, 6},
        UniformBlockInfo{"LinePatternInterpolationUBO", idLinePatternInterpolationUBO, 7},
        UniformBlockInfo{"LinePatternTilePropertiesUBO", idLinePatternTilePropertiesUBO, 8},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineSDFShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO, 9},
    UniformBlockInfo{"LineSDFUBO", idLineSDFUBO, 10},
    UniformBlockInfo{"LineSDFPropertiesUBO", idLineSDFPropertiesUBO, 11},
    UniformBlockInfo{"LineSDFInterpolationUBO", idLineSDFInterpolationUBO, 12},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO, 13},
    UniformBlockInfo{"LineUBO", idLineUBO, 14},
    UniformBlockInfo{"LinePropertiesUBO", idLinePropertiesUBO, 15},
    UniformBlockInfo{"LineInterpolationUBO", idLineInterpolationUBO, 16},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineBasicShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineBasicUBO", idLineBasicUBO, 17},
    UniformBlockInfo{"LineBasicPropertiesUBO", idLineBasicPropertiesUBO, 18},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"RasterDrawableUBO", idRasterDrawableUBO, 0},
};

const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO, 0},
    UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO, 1},
    UniformBlockInfo{"SymbolDrawablePaintUBO", idSymbolDrawablePaintUBO, 2},
    UniformBlockInfo{"SymbolDrawableTilePropsUBO", idSymbolDrawableTilePropsUBO, 3},
    UniformBlockInfo{"SymbolDrawableInterpolateUBO", idSymbolDrawableInterpolateUBO, 4},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO, 5},
        UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO, 6},
        UniformBlockInfo{"SymbolDrawablePaintUBO", idSymbolDrawablePaintUBO, 7},
        UniformBlockInfo{"SymbolDrawableTilePropsUBO", idSymbolDrawableTilePropsUBO, 8},
        UniformBlockInfo{"SymbolDrawableInterpolateUBO", idSymbolDrawableInterpolateUBO, 9},
};

const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO, 10},
        UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO, 11},
        UniformBlockInfo{"SymbolDrawablePaintUBO", idSymbolDrawablePaintUBO, 12},
        UniformBlockInfo{"SymbolDrawableTilePropsUBO", idSymbolDrawableTilePropsUBO, 13},
        UniformBlockInfo{"SymbolDrawableInterpolateUBO", idSymbolDrawableInterpolateUBO, 14},
};

} // namespace shaders
} // namespace mbgl
