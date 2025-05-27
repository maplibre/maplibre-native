#include <mbgl/shaders/gl/shader_info.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

UniformBlockInfo::UniformBlockInfo(std::string_view name_, std::size_t id_)
    : name(name_),
      id(id_),
      binding(id_) {}

AttributeInfo::AttributeInfo(std::string_view name_, std::size_t id_)
    : name(name_),
      id(id_) {}

TextureInfo::TextureInfo(std::string_view name_, std::size_t id_)
    : name(name_),
      id(id_) {}

// Background
using BackgroundShaderInfo = ShaderInfo<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> BackgroundShaderInfo::uniformBlocks = {
    UniformBlockInfo{"BackgroundDrawableUBO", idBackgroundDrawableUBO},
    UniformBlockInfo{"BackgroundPropsUBO", idBackgroundPropsUBO},
};
const std::vector<AttributeInfo> BackgroundShaderInfo::attributes = {
    AttributeInfo{"a_pos", idBackgroundPosVertexAttribute},
};
const std::vector<TextureInfo> BackgroundShaderInfo::textures = {};

// Background Pattern
using BackgroundPatternShaderInfo = ShaderInfo<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> BackgroundPatternShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"BackgroundPatternDrawableUBO", idBackgroundDrawableUBO},
    UniformBlockInfo{"BackgroundPatternPropsUBO", idBackgroundPropsUBO},
};
const std::vector<AttributeInfo> BackgroundPatternShaderInfo::attributes = {
    AttributeInfo{"a_pos", idBackgroundPosVertexAttribute},
};
const std::vector<TextureInfo> BackgroundPatternShaderInfo::textures = {
    TextureInfo{"u_image", idBackgroundImageTexture},
};

// Circle
using CircleShaderInfo = ShaderInfo<BuiltIn::CircleShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> CircleShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"CircleDrawableUBO", idCircleDrawableUBO},
    UniformBlockInfo{"CircleEvaluatedPropsUBO", idCircleEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> CircleShaderInfo::attributes = {
    AttributeInfo{"a_pos", idCirclePosVertexAttribute},
    AttributeInfo{"a_color", idCircleColorVertexAttribute},
    AttributeInfo{"a_radius", idCircleRadiusVertexAttribute},
    AttributeInfo{"a_blur", idCircleBlurVertexAttribute},
    AttributeInfo{"a_opacity", idCircleOpacityVertexAttribute},
    AttributeInfo{"a_stroke_color", idCircleStrokeColorVertexAttribute},
    AttributeInfo{"a_stroke_width", idCircleStrokeWidthVertexAttribute},
    AttributeInfo{"a_stroke_opacity", idCircleStrokeOpacityVertexAttribute},
};
const std::vector<TextureInfo> CircleShaderInfo::textures = {};

// Collision Box
using CollisionBoxShaderInfo = ShaderInfo<BuiltIn::CollisionBoxShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> CollisionBoxShaderInfo::uniformBlocks = {
    UniformBlockInfo{"CollisionDrawableUBO", idCollisionDrawableUBO},
    UniformBlockInfo{"CollisionTilePropsUBO", idCollisionTilePropsUBO},
};
const std::vector<AttributeInfo> CollisionBoxShaderInfo::attributes = {
    AttributeInfo{"a_pos", idCollisionPosVertexAttribute},
    AttributeInfo{"a_anchor_pos", idCollisionAnchorPosVertexAttribute},
    AttributeInfo{"a_extrude", idCollisionExtrudeVertexAttribute},
    AttributeInfo{"a_placed", idCollisionPlacedVertexAttribute},
    AttributeInfo{"a_shift", idCollisionShiftVertexAttribute},
};
const std::vector<TextureInfo> CollisionBoxShaderInfo::textures = {};

// Collision Circle
using CollisionCircleShaderInfo = ShaderInfo<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> CollisionCircleShaderInfo::uniformBlocks = {
    UniformBlockInfo{"CollisionDrawableUBO", idCollisionDrawableUBO},
    UniformBlockInfo{"CollisionTilePropsUBO", idCollisionTilePropsUBO},
};
const std::vector<AttributeInfo> CollisionCircleShaderInfo::attributes = {
    AttributeInfo{"a_pos", idCollisionPosVertexAttribute},
    AttributeInfo{"a_anchor_pos", idCollisionAnchorPosVertexAttribute},
    AttributeInfo{"a_extrude", idCollisionExtrudeVertexAttribute},
    AttributeInfo{"a_placed", idCollisionPlacedVertexAttribute},
};
const std::vector<TextureInfo> CollisionCircleShaderInfo::textures = {};

// Custom Geometry
using CustomGeometryInfo = ShaderInfo<BuiltIn::CustomGeometryShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> CustomGeometryInfo::uniformBlocks = {
    UniformBlockInfo{"CustomGeometryDrawableUBO", idCustomGeometryDrawableUBO},
};
const std::vector<AttributeInfo> CustomGeometryInfo::attributes = {
    AttributeInfo{"a_pos", idCustomGeometryPosVertexAttribute},
    AttributeInfo{"a_uv", idCustomGeometryTexVertexAttribute},
};
const std::vector<TextureInfo> CustomGeometryInfo::textures = {
    TextureInfo{"u_image", idCustomGeometryTexture},
};

// Custom Symbol Icon
using CustomSymbolIconShaderInfo = ShaderInfo<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> CustomSymbolIconShaderInfo::uniformBlocks = {
    UniformBlockInfo{"CustomSymbolIconDrawableUBO", idCustomSymbolDrawableUBO},
};
const std::vector<AttributeInfo> CustomSymbolIconShaderInfo::attributes = {
    AttributeInfo{"a_pos", idCustomSymbolPosVertexAttribute},
    AttributeInfo{"a_tex", idCustomSymbolTexVertexAttribute},
};
const std::vector<TextureInfo> CustomSymbolIconShaderInfo::textures = {
    TextureInfo{"u_texture", idCustomSymbolImageTexture},
};

// Debug
using DebugShaderInfo = ShaderInfo<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> DebugShaderInfo::uniformBlocks = {
    UniformBlockInfo{"DebugUBO", idDebugUBO},
};
const std::vector<AttributeInfo> DebugShaderInfo::attributes = {
    AttributeInfo{"a_pos", idDebugPosVertexAttribute},
};
const std::vector<TextureInfo> DebugShaderInfo::textures = {
    TextureInfo{"u_overlay", idDebugOverlayTexture},
};

// Fill
using FillShaderInfo = ShaderInfo<BuiltIn::FillShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> FillShaderInfo::uniformBlocks = {
    UniformBlockInfo{"FillDrawableUBO", idFillDrawableUBO},
    UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> FillShaderInfo::attributes = {
    AttributeInfo{"a_pos", idFillPosVertexAttribute},
    AttributeInfo{"a_color", idFillColorVertexAttribute},
    AttributeInfo{"a_opacity", idFillOpacityVertexAttribute},
};
const std::vector<TextureInfo> FillShaderInfo::textures = {};

// Fill Outline
using FillOutlineShaderInfo = ShaderInfo<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> FillOutlineShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"FillOutlineDrawableUBO", idFillDrawableUBO},
    UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> FillOutlineShaderInfo::attributes = {
    AttributeInfo{"a_pos", idFillPosVertexAttribute},
    AttributeInfo{"a_outline_color", idFillOutlineColorVertexAttribute},
    AttributeInfo{"a_opacity", idFillOpacityVertexAttribute},
};
const std::vector<TextureInfo> FillOutlineShaderInfo::textures = {};

// Fill Pattern
using FillPatternShaderInfo = ShaderInfo<BuiltIn::FillPatternShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> FillPatternShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"FillPatternDrawableUBO", idFillDrawableUBO},
    UniformBlockInfo{"FillPatternTilePropsUBO", idFillTilePropsUBO},
    UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> FillPatternShaderInfo::attributes = {
    AttributeInfo{"a_pos", idFillPosVertexAttribute},
    AttributeInfo{"a_opacity", idFillOpacityVertexAttribute},
    AttributeInfo{"a_pattern_from", idFillPatternFromVertexAttribute},
    AttributeInfo{"a_pattern_to", idFillPatternToVertexAttribute},
};
const std::vector<TextureInfo> FillPatternShaderInfo::textures = {
    TextureInfo{"u_image", idFillImageTexture},
};

// Fill Outline Pattern
using FillOutlinePatternShaderInfo = ShaderInfo<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> FillOutlinePatternShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"FillOutlinePatternDrawableUBO", idFillDrawableUBO},
    UniformBlockInfo{"FillOutlinePatternTilePropsUBO", idFillTilePropsUBO},
    UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> FillOutlinePatternShaderInfo::attributes = {
    AttributeInfo{"a_pos", idFillPosVertexAttribute},
    AttributeInfo{"a_opacity", idFillOpacityVertexAttribute},
    AttributeInfo{"a_pattern_from", idFillPatternFromVertexAttribute},
    AttributeInfo{"a_pattern_to", idFillPatternToVertexAttribute},
};
const std::vector<TextureInfo> FillOutlinePatternShaderInfo::textures = {
    TextureInfo{"u_image", idFillImageTexture},
};

// Fill Outline Triangulated
using FillOutlineTriangulatedShaderInfo =
    ShaderInfo<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> FillOutlineTriangulatedShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"FillOutlineTriangulatedDrawableUBO", idFillDrawableUBO},
    UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> FillOutlineTriangulatedShaderInfo::attributes = {
    AttributeInfo{"a_pos_normal", idLinePosNormalVertexAttribute},
    AttributeInfo{"a_data", idLineDataVertexAttribute},
};
const std::vector<TextureInfo> FillOutlineTriangulatedShaderInfo::textures = {};

// Fill Extrusion
using FillExtrusionShaderInfo = ShaderInfo<BuiltIn::FillExtrusionShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> FillExtrusionShaderInfo::uniformBlocks = {
    UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO},
    UniformBlockInfo{"FillExtrusionTilePropsUBO", idFillExtrusionTilePropsUBO},
    UniformBlockInfo{"FillExtrusionPropsUBO", idFillExtrusionPropsUBO},
};
const std::vector<AttributeInfo> FillExtrusionShaderInfo::attributes = {
    AttributeInfo{"a_pos", idFillExtrusionPosVertexAttribute},
    AttributeInfo{"a_normal_ed", idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{"a_base", idFillExtrusionBaseVertexAttribute},
    AttributeInfo{"a_height", idFillExtrusionHeightVertexAttribute},
    AttributeInfo{"a_color", idFillExtrusionColorVertexAttribute},
};
const std::vector<TextureInfo> FillExtrusionShaderInfo::textures = {};

// Fill Extrusion Pattern
using FillExtrusionPatternShaderInfo = ShaderInfo<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> FillExtrusionPatternShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO},
    UniformBlockInfo{"FillExtrusionTilePropsUBO", idFillExtrusionTilePropsUBO},
    UniformBlockInfo{"FillExtrusionPropsUBO", idFillExtrusionPropsUBO},
};
const std::vector<AttributeInfo> FillExtrusionPatternShaderInfo::attributes = {
    AttributeInfo{"a_pos", idFillExtrusionPosVertexAttribute},
    AttributeInfo{"a_normal_ed", idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{"a_base", idFillExtrusionBaseVertexAttribute},
    AttributeInfo{"a_height", idFillExtrusionHeightVertexAttribute},
    AttributeInfo{"a_pattern_from", idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{"a_pattern_to", idFillExtrusionPatternToVertexAttribute},
};
const std::vector<TextureInfo> FillExtrusionPatternShaderInfo::textures = {
    TextureInfo{"u_image", idFillExtrusionImageTexture},
};

// Heatmap
using HeatmapShaderInfo = ShaderInfo<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> HeatmapShaderInfo::uniformBlocks = {
    UniformBlockInfo{"HeatmapDrawableUBO", idHeatmapDrawableUBO},
    UniformBlockInfo{"HeatmapEvaluatedPropsUBO", idHeatmapEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> HeatmapShaderInfo::attributes = {
    AttributeInfo{"a_pos", idHeatmapPosVertexAttribute},
    AttributeInfo{"a_weight", idHeatmapWeightVertexAttribute},
    AttributeInfo{"a_radius", idHeatmapRadiusVertexAttribute},
};
const std::vector<TextureInfo> HeatmapShaderInfo::textures = {};

// Heatmap Texture
using HeatmapTextureShaderInfo = ShaderInfo<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> HeatmapTextureShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"HeatmapTexturePropsUBO", idHeatmapTexturePropsUBO},
};
const std::vector<AttributeInfo> HeatmapTextureShaderInfo::attributes = {
    AttributeInfo{"a_pos", idHeatmapPosVertexAttribute},
};
const std::vector<TextureInfo> HeatmapTextureShaderInfo::textures = {
    TextureInfo{"u_image", idHeatmapImageTexture},
    TextureInfo{"u_color_ramp", idHeatmapColorRampTexture},
};

// Hillshade Prepare
using HillshadePrepareShaderInfo = ShaderInfo<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> HillshadePrepareShaderInfo::uniformBlocks = {
    UniformBlockInfo{"HillshadePrepareDrawableUBO", idHillshadePrepareDrawableUBO},
    UniformBlockInfo{"HillshadePrepareTilePropsUBO", idHillshadePrepareTilePropsUBO},
};
const std::vector<AttributeInfo> HillshadePrepareShaderInfo::attributes = {
    AttributeInfo{"a_pos", idHillshadePosVertexAttribute},
    AttributeInfo{"a_texture_pos", idHillshadeTexturePosVertexAttribute},
};
const std::vector<TextureInfo> HillshadePrepareShaderInfo::textures = {
    TextureInfo{"u_image", idHillshadeImageTexture},
};

// Hillshade
using HillshadeShaderInfo = ShaderInfo<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> HillshadeShaderInfo::uniformBlocks = {
    UniformBlockInfo{"HillshadeDrawableUBO", idHillshadeDrawableUBO},
    UniformBlockInfo{"HillshadeTilePropsUBO", idHillshadeTilePropsUBO},
    UniformBlockInfo{"HillshadeEvaluatedPropsUBO", idHillshadeEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> HillshadeShaderInfo::attributes = {
    AttributeInfo{"a_pos", idHillshadePosVertexAttribute},
    AttributeInfo{"a_texture_pos", idHillshadeTexturePosVertexAttribute},
};
const std::vector<TextureInfo> HillshadeShaderInfo::textures = {
    TextureInfo{"u_image", idHillshadeImageTexture},
};

// Line
using LineShaderInfo = ShaderInfo<BuiltIn::LineShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> LineShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"LineDrawableUBO", idLineDrawableUBO},
    UniformBlockInfo{"LineEvaluatedPropsUBO", idLineEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> LineShaderInfo::attributes = {
    AttributeInfo{"a_pos_normal", idLinePosNormalVertexAttribute},
    AttributeInfo{"a_data", idLineDataVertexAttribute},
    AttributeInfo{"a_color", idLineColorVertexAttribute},
    AttributeInfo{"a_blur", idLineBlurVertexAttribute},
    AttributeInfo{"a_opacity", idLineOpacityVertexAttribute},
    AttributeInfo{"a_gapwidth", idLineGapWidthVertexAttribute},
    AttributeInfo{"a_offset", idLineOffsetVertexAttribute},
    AttributeInfo{"a_width", idLineWidthVertexAttribute},
};
const std::vector<TextureInfo> LineShaderInfo::textures = {};

// Location Indicator
using LocationIndicatorInfo = ShaderInfo<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> LocationIndicatorInfo::uniformBlocks = {
    UniformBlockInfo{"LocationIndicatorDrawableUBO", idLocationIndicatorDrawableUBO},
};
const std::vector<AttributeInfo> LocationIndicatorInfo::attributes = {
    AttributeInfo{"a_pos", idLocationIndicatorPosVertexAttribute},
};
const std::vector<TextureInfo> LocationIndicatorInfo::textures = {};

using LocationIndicatorTexturedInfo = ShaderInfo<BuiltIn::LocationIndicatorTexturedShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> LocationIndicatorTexturedInfo::uniformBlocks = {
    UniformBlockInfo{"LocationIndicatorDrawableUBO", idLocationIndicatorDrawableUBO},
};
const std::vector<AttributeInfo> LocationIndicatorTexturedInfo::attributes = {
    AttributeInfo{"a_pos", idLocationIndicatorPosVertexAttribute},
    AttributeInfo{"a_uv", idLocationIndicatorTexVertexAttribute},
};
const std::vector<TextureInfo> LocationIndicatorTexturedInfo::textures = {
    TextureInfo{"u_image", idLocationIndicatorTexture},
};

// Line Gradient
using LineGradientShaderInfo = ShaderInfo<BuiltIn::LineGradientShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> LineGradientShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"LineGradientDrawableUBO", idLineDrawableUBO},
    UniformBlockInfo{"LineEvaluatedPropsUBO", idLineEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> LineGradientShaderInfo::attributes = {
    AttributeInfo{"a_pos_normal", idLinePosNormalVertexAttribute},
    AttributeInfo{"a_data", idLineDataVertexAttribute},
    AttributeInfo{"a_blur", idLineBlurVertexAttribute},
    AttributeInfo{"a_opacity", idLineOpacityVertexAttribute},
    AttributeInfo{"a_gapwidth", idLineGapWidthVertexAttribute},
    AttributeInfo{"a_offset", idLineOffsetVertexAttribute},
    AttributeInfo{"a_width", idLineWidthVertexAttribute},
};
const std::vector<TextureInfo> LineGradientShaderInfo::textures = {
    TextureInfo{"u_image", idLineImageTexture},
};

// Line Pattern
using LinePatternShaderInfo = ShaderInfo<BuiltIn::LinePatternShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> LinePatternShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"LinePatternDrawableUBO", idLineDrawableUBO},
    UniformBlockInfo{"LinePatternTilePropsUBO", idLineTilePropsUBO},
    UniformBlockInfo{"LineEvaluatedPropsUBO", idLineEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> LinePatternShaderInfo::attributes = {
    AttributeInfo{"a_pos_normal", idLinePosNormalVertexAttribute},
    AttributeInfo{"a_data", idLineDataVertexAttribute},
    AttributeInfo{"a_blur", idLineBlurVertexAttribute},
    AttributeInfo{"a_opacity", idLineOpacityVertexAttribute},
    AttributeInfo{"a_offset", idLineOffsetVertexAttribute},
    AttributeInfo{"a_gapwidth", idLineGapWidthVertexAttribute},
    AttributeInfo{"a_width", idLineWidthVertexAttribute},
    AttributeInfo{"a_pattern_from", idLinePatternFromVertexAttribute},
    AttributeInfo{"a_pattern_to", idLinePatternToVertexAttribute},
};
const std::vector<TextureInfo> LinePatternShaderInfo::textures = {
    TextureInfo{"u_image", idLineImageTexture},
};

// Line SDF
using LineSDFShaderInfo = ShaderInfo<BuiltIn::LineSDFShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> LineSDFShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"LineSDFDrawableUBO", idLineDrawableUBO},
    UniformBlockInfo{"LineSDFTilePropsUBO", idLineTilePropsUBO},
    UniformBlockInfo{"LineEvaluatedPropsUBO", idLineEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> LineSDFShaderInfo::attributes = {
    AttributeInfo{"a_pos_normal", idLinePosNormalVertexAttribute},
    AttributeInfo{"a_data", idLineDataVertexAttribute},
    AttributeInfo{"a_color", idLineColorVertexAttribute},
    AttributeInfo{"a_blur", idLineBlurVertexAttribute},
    AttributeInfo{"a_opacity", idLineOpacityVertexAttribute},
    AttributeInfo{"a_gapwidth", idLineGapWidthVertexAttribute},
    AttributeInfo{"a_offset", idLineOffsetVertexAttribute},
    AttributeInfo{"a_width", idLineWidthVertexAttribute},
    AttributeInfo{"a_floorwidth", idLineFloorWidthVertexAttribute},
};
const std::vector<TextureInfo> LineSDFShaderInfo::textures = {
    TextureInfo{"u_image", idLineImageTexture},
};

// Raster
using RasterShaderInfo = ShaderInfo<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> RasterShaderInfo::uniformBlocks = {
    UniformBlockInfo{"RasterDrawableUBO", idRasterDrawableUBO},
    UniformBlockInfo{"RasterEvaluatedPropsUBO", idRasterEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> RasterShaderInfo::attributes = {
    AttributeInfo{"a_pos", idRasterPosVertexAttribute},
    AttributeInfo{"a_texture_pos", idRasterTexturePosVertexAttribute},
};
const std::vector<TextureInfo> RasterShaderInfo::textures = {
    TextureInfo{"u_image0", idRasterImage0Texture},
    TextureInfo{"u_image1", idRasterImage0Texture},
};

// Symbol Icon
using SymbolIconShaderInfo = ShaderInfo<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> SymbolIconShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
    UniformBlockInfo{"SymbolTilePropsUBO", idSymbolTilePropsUBO},
    UniformBlockInfo{"SymbolEvaluatedPropsUBO", idSymbolEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> SymbolIconShaderInfo::attributes = {
    AttributeInfo{"a_pos_offset", idSymbolPosOffsetVertexAttribute},
    AttributeInfo{"a_data", idSymbolDataVertexAttribute},
    AttributeInfo{"a_pixeloffset", idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{"a_projected_pos", idSymbolProjectedPosVertexAttribute},
    AttributeInfo{"a_fade_opacity", idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{"a_opacity", idSymbolOpacityVertexAttribute},
};
const std::vector<TextureInfo> SymbolIconShaderInfo::textures = {
    TextureInfo{"u_texture", idSymbolImageTexture},
};

// Symbol SDF
using SymbolSDFShaderInfo = ShaderInfo<BuiltIn::SymbolSDFShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> SymbolSDFShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
    UniformBlockInfo{"SymbolTilePropsUBO", idSymbolTilePropsUBO},
    UniformBlockInfo{"SymbolEvaluatedPropsUBO", idSymbolEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> SymbolSDFShaderInfo::attributes = {
    AttributeInfo{"a_pos_offset", idSymbolPosOffsetVertexAttribute},
    AttributeInfo{"a_data", idSymbolDataVertexAttribute},
    AttributeInfo{"a_pixeloffset", idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{"a_projected_pos", idSymbolProjectedPosVertexAttribute},
    AttributeInfo{"a_fade_opacity", idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{"a_fill_color", idSymbolColorVertexAttribute},
    AttributeInfo{"a_halo_color", idSymbolHaloColorVertexAttribute},
    AttributeInfo{"a_opacity", idSymbolOpacityVertexAttribute},
    AttributeInfo{"a_halo_width", idSymbolHaloWidthVertexAttribute},
    AttributeInfo{"a_halo_blur", idSymbolHaloBlurVertexAttribute},
};
const std::vector<TextureInfo> SymbolSDFShaderInfo::textures = {
    TextureInfo{"u_texture", idSymbolImageTexture},
};

// Symbol Text & Icon
using SymbolTextAndIconShaderInfo = ShaderInfo<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::OpenGL>;

const std::vector<UniformBlockInfo> SymbolTextAndIconShaderInfo::uniformBlocks = {
    UniformBlockInfo{"GlobalPaintParamsUBO", idGlobalPaintParamsUBO},
    UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
    UniformBlockInfo{"SymbolTilePropsUBO", idSymbolTilePropsUBO},
    UniformBlockInfo{"SymbolEvaluatedPropsUBO", idSymbolEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> SymbolTextAndIconShaderInfo::attributes = {
    AttributeInfo{"a_pos_offset", idSymbolPosOffsetVertexAttribute},
    AttributeInfo{"a_data", idSymbolDataVertexAttribute},
    AttributeInfo{"a_projected_pos", idSymbolProjectedPosVertexAttribute},
    AttributeInfo{"a_fade_opacity", idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{"a_fill_color", idSymbolColorVertexAttribute},
    AttributeInfo{"a_halo_color", idSymbolHaloColorVertexAttribute},
    AttributeInfo{"a_opacity", idSymbolOpacityVertexAttribute},
    AttributeInfo{"a_halo_width", idSymbolHaloWidthVertexAttribute},
    AttributeInfo{"a_halo_blur", idSymbolHaloBlurVertexAttribute},
};
const std::vector<TextureInfo> SymbolTextAndIconShaderInfo::textures = {
    TextureInfo{"u_texture", idSymbolImageTexture},
    TextureInfo{"u_texture_icon", idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
