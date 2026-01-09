#pragma once

#include <mbgl/shaders/shader_source.hpp>

#include <vector>
#include <string>

namespace mbgl {
namespace shaders {

struct UniformBlockInfo {
    UniformBlockInfo(std::string_view name, std::size_t id);
    std::string_view name;
    std::size_t id;
    std::size_t binding;
};

struct AttributeInfo {
    AttributeInfo(std::string_view name, std::size_t id);
    std::string_view name;
    std::size_t id;
};

struct TextureInfo {
    TextureInfo(std::string_view name, std::size_t id);
    std::string_view name;
    std::size_t id;
};

template <BuiltIn T, gfx::Backend::Type>
struct ShaderInfo;

template <>
struct ShaderInfo<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::CircleShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::CollisionBoxShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::CustomGeometryShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::FillShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::FillPatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::FillExtrusionShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::LineGradientShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::LinePatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::LineSDFShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::LineShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::LocationIndicatorTexturedShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::SymbolSDFShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

template <>
struct ShaderInfo<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<AttributeInfo> attributes;
    static const std::vector<UniformBlockInfo> uniformBlocks;
    static const std::vector<TextureInfo> textures;
};

} // namespace shaders
} // namespace mbgl
