#pragma once

#include <mbgl/gl/context.hpp>
#include <mbgl/gl/uniform_block_gl.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/util/string_indexer.hpp>

#include <unordered_map>

namespace mbgl {

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
