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
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"BackgroundDrawableUBO", idBackgroundDrawableUBO},
    UniformBlockInfo{"BackgroundLayerUBO", idBackgroundLayerUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idBackgroundPosVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL>::textures = {};

// Background Pattern
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"BackgroundPatternDrawableUBO", idBackgroundDrawableUBO},
        UniformBlockInfo{"BackgroundPatternLayerUBO", idBackgroundLayerUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL>::attributes =
    {
        AttributeInfo{"a_pos", idBackgroundPosVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idBackgroundImageTexture},
};

// Circle
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::CircleShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"CircleDrawableUBO", idCircleDrawableUBO},
    UniformBlockInfo{"CirclePaintParamsUBO", idCirclePaintParamsUBO},
    UniformBlockInfo{"CircleEvaluatedPropsUBO", idCircleEvaluatedPropsUBO},
    UniformBlockInfo{"CircleInterpolateUBO", idCircleInterpolateUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::CircleShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idCirclePosVertexAttribute},
    AttributeInfo{"a_color", idCircleColorVertexAttribute},
    AttributeInfo{"a_radius", idCircleRadiusVertexAttribute},
    AttributeInfo{"a_blur", idCircleBlurVertexAttribute},
    AttributeInfo{"a_opacity", idCircleOpacityVertexAttribute},
    AttributeInfo{"a_stroke_color", idCircleStrokeColorVertexAttribute},
    AttributeInfo{"a_stroke_width", idCircleStrokeWidthVertexAttribute},
    AttributeInfo{"a_stroke_opacity", idCircleStrokeOpacityVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::CircleShader, gfx::Backend::Type::OpenGL>::textures = {};

// Collision Box
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::CollisionBoxShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"CollisionBoxUBO", idCollisionUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::CollisionBoxShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idCollisionPosVertexAttribute},
    AttributeInfo{"a_anchor_pos", idCollisionAnchorPosVertexAttribute},
    AttributeInfo{"a_extrude", idCollisionExtrudeVertexAttribute},
    AttributeInfo{"a_placed", idCollisionPlacedVertexAttribute},
    AttributeInfo{"a_shift", idCollisionShiftVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::CollisionBoxShader, gfx::Backend::Type::OpenGL>::textures = {};

// Collision Circle
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"CollisionCircleUBO", idCollisionUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idCollisionPosVertexAttribute},
    AttributeInfo{"a_anchor_pos", idCollisionAnchorPosVertexAttribute},
    AttributeInfo{"a_extrude", idCollisionExtrudeVertexAttribute},
    AttributeInfo{"a_placed", idCollisionPlacedVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL>::textures = {};

// Custom Symbol Icon
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"CustomSymbolIconDrawableUBO", idCustomSymbolDrawableUBO},
        UniformBlockInfo{"CustomSymbolIconParametersUBO", idCustomSymbolParametersUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idCustomSymbolPosVertexAttribute},
    AttributeInfo{"a_tex", idCustomSymbolTexVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_texture", idCustomSymbolImageTexture},
};

// Debug
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"DebugUBO", idDebugUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idDebugPosVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_overlay", idDebugOverlayTexture},
};

// Fill
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"FillDrawableUBO", idFillDrawableUBO},
    UniformBlockInfo{"FillInterpolateUBO", idFillInterpolateUBO},
    UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::FillShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idFillPosVertexAttribute},
    AttributeInfo{"a_color", idFillColorVertexAttribute},
    AttributeInfo{"a_opacity", idFillOpacityVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::FillShader, gfx::Backend::Type::OpenGL>::textures = {};

// Fill Outline
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"FillOutlineDrawableUBO", idFillDrawableUBO},
        UniformBlockInfo{"FillOutlineInterpolateUBO", idFillInterpolateUBO},
        UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idFillPosVertexAttribute},
    AttributeInfo{"a_outline_color", idFillOutlineColorVertexAttribute},
    AttributeInfo{"a_opacity", idFillOpacityVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL>::textures = {};

// Fill Pattern
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::FillPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"FillPatternDrawableUBO", idFillDrawableUBO},
        UniformBlockInfo{"FillPatternTilePropsUBO", idFillTilePropsUBO},
        UniformBlockInfo{"FillPatternInterpolateUBO", idFillInterpolateUBO},
        UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::FillPatternShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idFillPosVertexAttribute},
    AttributeInfo{"a_opacity", idFillOpacityVertexAttribute},
    AttributeInfo{"a_pattern_from", idFillPatternFromVertexAttribute},
    AttributeInfo{"a_pattern_to", idFillPatternToVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::FillPatternShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idFillImageTexture},
};

// Fill Outline Pattern
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillOutlinePatternDrawableUBO", idFillDrawableUBO},
        UniformBlockInfo{"FillOutlinePatternTilePropsUBO", idFillTilePropsUBO},
        UniformBlockInfo{"FillOutlinePatternInterpolateUBO", idFillInterpolateUBO},
        UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::OpenGL>::attributes =
    {
        AttributeInfo{"a_pos", idFillPosVertexAttribute},
        AttributeInfo{"a_opacity", idFillOpacityVertexAttribute},
        AttributeInfo{"a_pattern_from", idFillPatternFromVertexAttribute},
        AttributeInfo{"a_pattern_to", idFillPatternToVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idFillImageTexture},
};

// Fill Outline Triangulated
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillOutlineTriangulatedDrawableUBO", idFillDrawableUBO},
        UniformBlockInfo{"FillEvaluatedPropsUBO", idFillEvaluatedPropsUBO},
};
const std::vector<AttributeInfo>
    ShaderInfo<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::OpenGL>::attributes = {
        AttributeInfo{"a_pos_normal", idLinePosNormalVertexAttribute},
        AttributeInfo{"a_data", idLineDataVertexAttribute},
};
const std::vector<TextureInfo>
    ShaderInfo<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::OpenGL>::textures = {};

// Fill Extrusion
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillExtrusionShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO},
        UniformBlockInfo{"FillExtrusionPropsUBO", idFillExtrusionPropsUBO},
        UniformBlockInfo{"FillExtrusionTilePropsUBO", idFillExtrusionTilePropsUBO},
        UniformBlockInfo{"FillExtrusionInterpolateUBO", idFillExtrusionInterpolateUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::FillExtrusionShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idFillExtrusionPosVertexAttribute},
    AttributeInfo{"a_normal_ed", idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{"a_base", idFillExtrusionBaseVertexAttribute},
    AttributeInfo{"a_height", idFillExtrusionHeightVertexAttribute},
    AttributeInfo{"a_color", idFillExtrusionColorVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::FillExtrusionShader, gfx::Backend::Type::OpenGL>::textures = {};

// Fill Extrusion Pattern
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"FillExtrusionDrawableUBO", idFillExtrusionDrawableUBO},
        UniformBlockInfo{"FillExtrusionPropsUBO", idFillExtrusionPropsUBO},
        UniformBlockInfo{"FillExtrusionTilePropsUBO", idFillExtrusionTilePropsUBO},
        UniformBlockInfo{"FillExtrusionInterpolateUBO", idFillExtrusionInterpolateUBO},
};
const std::vector<AttributeInfo>
    ShaderInfo<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL>::attributes = {
        AttributeInfo{"a_pos", idFillExtrusionPosVertexAttribute},
        AttributeInfo{"a_normal_ed", idFillExtrusionNormalEdVertexAttribute},
        AttributeInfo{"a_base", idFillExtrusionBaseVertexAttribute},
        AttributeInfo{"a_height", idFillExtrusionHeightVertexAttribute},
        AttributeInfo{"a_pattern_from", idFillExtrusionPatternFromVertexAttribute},
        AttributeInfo{"a_pattern_to", idFillExtrusionPatternToVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idFillExtrusionImageTexture},
};

// Heatmap
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"HeatmapDrawableUBO", idHeatmapDrawableUBO},
    UniformBlockInfo{"HeatmapEvaluatedPropsUBO", idHeatmapEvaluatedPropsUBO},
    UniformBlockInfo{"HeatmapInterpolateUBO", idHeatmapInterpolateUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idHeatmapPosVertexAttribute},
    AttributeInfo{"a_weight", idHeatmapWeightVertexAttribute},
    AttributeInfo{"a_radius", idHeatmapRadiusVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL>::textures = {};

// Heatmap Texture
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"HeatmapTexturePropsUBO", idHeatmapTexturePropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idHeatmapPosVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idHeatmapImageTexture},
    TextureInfo{"u_color_ramp", idHeatmapColorRampTexture},
};

// Hillshade Prepare
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"HillshadePrepareDrawableUBO", idHillshadePrepareDrawableUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idHillshadePosVertexAttribute},
    AttributeInfo{"a_texture_pos", idHillshadeTexturePosVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idHillshadeImageTexture},
};

// Hillshade
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"HillshadeDrawableUBO", idHillshadeDrawableUBO},
    UniformBlockInfo{"HillshadeEvaluatedPropsUBO", idHillshadeEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idHillshadePosVertexAttribute},
    AttributeInfo{"a_texture_pos", idHillshadeTexturePosVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idHillshadeImageTexture},
};

// Line
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
    UniformBlockInfo{"LineDrawableUBO", idLineDrawableUBO},
    UniformBlockInfo{"LineInterpolationUBO", idLineInterpolationUBO},
    UniformBlockInfo{"LineEvaluatedPropsUBO", idLineEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::LineShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos_normal", idLinePosNormalVertexAttribute},
    AttributeInfo{"a_data", idLineDataVertexAttribute},
    AttributeInfo{"a_color", idLineColorVertexAttribute},
    AttributeInfo{"a_blur", idLineBlurVertexAttribute},
    AttributeInfo{"a_opacity", idLineOpacityVertexAttribute},
    AttributeInfo{"a_gapwidth", idLineGapWidthVertexAttribute},
    AttributeInfo{"a_offset", idLineOffsetVertexAttribute},
    AttributeInfo{"a_width", idLineWidthVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::LineShader, gfx::Backend::Type::OpenGL>::textures = {};

// Line Gradient
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineGradientShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
        UniformBlockInfo{"LineGradientDrawableUBO", idLineDrawableUBO},
        UniformBlockInfo{"LineGradientInterpolationUBO", idLineInterpolationUBO},
        UniformBlockInfo{"LineEvaluatedPropsUBO", idLineEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::LineGradientShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos_normal", idLinePosNormalVertexAttribute},
    AttributeInfo{"a_data", idLineDataVertexAttribute},
    AttributeInfo{"a_blur", idLineBlurVertexAttribute},
    AttributeInfo{"a_opacity", idLineOpacityVertexAttribute},
    AttributeInfo{"a_gapwidth", idLineGapWidthVertexAttribute},
    AttributeInfo{"a_offset", idLineOffsetVertexAttribute},
    AttributeInfo{"a_width", idLineWidthVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::LineGradientShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idLineImageTexture},
};

// Line Pattern
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LinePatternShader, gfx::Backend::Type::OpenGL>::uniformBlocks =
    {
        UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
        UniformBlockInfo{"LinePatternDrawableUBO", idLineDrawableUBO},
        UniformBlockInfo{"LinePatternInterpolationUBO", idLineInterpolationUBO},
        UniformBlockInfo{"LinePatternTilePropertiesUBO", idLineTilePropertiesUBO},
        UniformBlockInfo{"LineEvaluatedPropsUBO", idLineEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::LinePatternShader, gfx::Backend::Type::OpenGL>::attributes = {
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
const std::vector<TextureInfo> ShaderInfo<BuiltIn::LinePatternShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idLineImageTexture},
};

// Line SDF
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::LineSDFShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"LineDynamicUBO", idLineDynamicUBO},
    UniformBlockInfo{"LineSDFDrawableUBO", idLineDrawableUBO},
    UniformBlockInfo{"LineSDFInterpolationUBO", idLineInterpolationUBO},
    UniformBlockInfo{"LineEvaluatedPropsUBO", idLineEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::LineSDFShader, gfx::Backend::Type::OpenGL>::attributes = {
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
const std::vector<TextureInfo> ShaderInfo<BuiltIn::LineSDFShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image", idLineImageTexture},
};

// Raster
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"RasterDrawableUBO", idRasterDrawableUBO},
    UniformBlockInfo{"RasterEvaluatedPropsUBO", idRasterEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos", idRasterPosVertexAttribute},
    AttributeInfo{"a_texture_pos", idRasterTexturePosVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_image0", idRasterImage0Texture},
    TextureInfo{"u_image1", idRasterImage0Texture},
};

// Symbol Icon
const std::vector<UniformBlockInfo> ShaderInfo<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
    UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO},
    UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
    UniformBlockInfo{"SymbolTilePropsUBO", idSymbolTilePropsUBO},
    UniformBlockInfo{"SymbolInterpolateUBO", idSymbolInterpolateUBO},
    UniformBlockInfo{"SymbolEvaluatedPropsUBO", idSymbolEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL>::attributes = {
    AttributeInfo{"a_pos_offset", idSymbolPosOffsetVertexAttribute},
    AttributeInfo{"a_data", idSymbolDataVertexAttribute},
    AttributeInfo{"a_pixeloffset", idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{"a_projected_pos", idSymbolProjectedPosVertexAttribute},
    AttributeInfo{"a_fade_opacity", idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{"a_opacity", idSymbolOpacityVertexAttribute},
};
const std::vector<TextureInfo> ShaderInfo<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_texture", idSymbolImageTexture},
};

// Symbol SDF
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO},
        UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
        UniformBlockInfo{"SymbolTilePropsUBO", idSymbolTilePropsUBO},
        UniformBlockInfo{"SymbolInterpolateUBO", idSymbolInterpolateUBO},
        UniformBlockInfo{"SymbolEvaluatedPropsUBO", idSymbolEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::OpenGL>::attributes = {
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
const std::vector<TextureInfo> ShaderInfo<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_texture", idSymbolImageTexture},
};

// Symbol Text & Icon
const std::vector<UniformBlockInfo>
    ShaderInfo<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::OpenGL>::uniformBlocks = {
        UniformBlockInfo{"SymbolDynamicUBO", idSymbolDynamicUBO},
        UniformBlockInfo{"SymbolDrawableUBO", idSymbolDrawableUBO},
        UniformBlockInfo{"SymbolTilePropsUBO", idSymbolTilePropsUBO},
        UniformBlockInfo{"SymbolInterpolateUBO", idSymbolInterpolateUBO},
        UniformBlockInfo{"SymbolEvaluatedPropsUBO", idSymbolEvaluatedPropsUBO},
};
const std::vector<AttributeInfo> ShaderInfo<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::OpenGL>::attributes =
    {
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
const std::vector<TextureInfo> ShaderInfo<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::OpenGL>::textures = {
    TextureInfo{"u_texture", idSymbolImageTexture},
    TextureInfo{"u_texture_icon", idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
