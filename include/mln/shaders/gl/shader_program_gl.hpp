#pragma once

#include <mln/gl/context.hpp>
#include <mln/gl/vertex_attribute_gl.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/shaders/gl/shader_info.hpp>

#include <unordered_map>

namespace mln {

class ProgramParameters;

namespace gl {

class ShaderProgramGL final : public gfx::ShaderProgramBase {
public:
    using SamplerLocationArray = std::array<std::optional<size_t>, shaders::maxTextureCountPerShader>;

    ShaderProgramGL(UniqueProgram&& glProgram_);
    ShaderProgramGL(UniqueProgram&&, VertexAttributeArrayGL&& attributes, SamplerLocationArray&& samplerLocations);
    ShaderProgramGL(ShaderProgramGL&& other);
    ~ShaderProgramGL() noexcept override = default;

    static constexpr std::string_view Name{"GenericGLShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    static std::shared_ptr<ShaderProgramGL> create(Context&,
                                                   const ProgramParameters& programParameters,
                                                   const std::string_view firstAttribName,
                                                   const std::vector<shaders::UniformBlockInfo>& uniformBlocksInfo,
                                                   const std::vector<shaders::TextureInfo>& texturesInfo,
                                                   const std::vector<shaders::AttributeInfo>& attributesInfo,
                                                   const std::string& vertexSource,
                                                   const std::string& fragmentSource,
                                                   const std::string& additionalDefines = "") noexcept(false);

    std::optional<size_t> getSamplerLocation(const size_t id) const override;

    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }

    const gfx::VertexAttributeArray& getInstanceAttributes() const override { return instanceAttributes; }

    ProgramID getGLProgramID() const { return glProgram; }

protected:
    UniqueProgram glProgram;

    VertexAttributeArrayGL vertexAttributes;
    VertexAttributeArrayGL instanceAttributes;
    SamplerLocationArray samplerLocations;
};

} // namespace gl
} // namespace mln
