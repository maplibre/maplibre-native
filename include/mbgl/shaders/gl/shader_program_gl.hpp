#pragma once

#include <mbgl/gl/context.hpp>
#include <mbgl/gl/uniform_block_gl.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/util/string_indexer.hpp>

#include <unordered_map>

namespace mbgl {
namespace shaders {

struct UniformBlockInfo {
    UniformBlockInfo(std::string_view name, std::size_t binding);
    std::string_view name;
    std::size_t binding;
};

template <BuiltIn T, gfx::Backend::Type>
struct ShaderInfo;

template <>
struct ShaderInfo<BuiltIn::BackgroundShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::CircleShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::CollisionBoxShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::CollisionCircleShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::FillShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::LineGradientShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::LinePatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::LineSDFShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::LineShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::LineBasicShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::FillPatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::FillExtrusionShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::RasterShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};

template <>
struct ShaderInfo<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::OpenGL> {
    static const std::vector<UniformBlockInfo> uniformBlocks;
};
} // namespace shaders

class ProgramParameters;

namespace gl {

class ShaderProgramGL final : public gfx::ShaderProgramBase {
public:
    using SamplerLocationMap = std::unordered_map<StringIdentity, int>;

    ShaderProgramGL(UniqueProgram&& glProgram_);
    ShaderProgramGL(UniqueProgram&&,
                    UniformBlockArrayGL&& uniformBlocks,
                    VertexAttributeArrayGL&& attributes,
                    SamplerLocationMap&& samplerLocations);
    ShaderProgramGL(ShaderProgramGL&& other);
    ~ShaderProgramGL() noexcept override = default;

    static constexpr std::string_view Name{"GenericGLShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    static std::shared_ptr<ShaderProgramGL> create(Context&,
                                                   const ProgramParameters& programParameters,
                                                   const std::string& name,
                                                   const std::string_view firstAttribName,
                                                   const std::vector<shaders::UniformBlockInfo>& uniformBlocksInfo,
                                                   const std::string& vertexSource,
                                                   const std::string& fragmentSource,
                                                   const std::string& additionalDefines = "") noexcept(false);

    std::optional<uint32_t> getSamplerLocation(const StringIdentity id) const override;

    const gfx::UniformBlockArray& getUniformBlocks() const override { return uniformBlocks; }

    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }

    ProgramID getGLProgramID() const { return glProgram; }

protected:
    gfx::UniformBlockArray& mutableUniformBlocks() override { return uniformBlocks; }

protected:
    UniqueProgram glProgram;

    UniformBlockArrayGL uniformBlocks;
    VertexAttributeArrayGL vertexAttributes;
    SamplerLocationMap samplerLocations;
};

} // namespace gl
} // namespace mbgl
