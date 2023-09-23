#pragma once

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>
#include <mbgl/mtl/uniform_block.hpp>
#include <mbgl/mtl/vertex_attribute.hpp>

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLLibrary.hpp>

#include <optional>
#include <string>
#include <vector>

namespace mbgl {
namespace shaders {
struct AttributeInfo {
    std::size_t index;
    gfx::AttributeDataType dataType;
    std::size_t count;
    std::string_view name;
};
struct UniformBlockInfo {
    std::size_t index;
    bool vertex;
    bool fragment;
    std::size_t size;
    std::string_view name;
};
struct TextureInfo {
    std::size_t index;
    std::string_view name;
};
} // namespace shaders
namespace mtl {
class RenderableResource;
class RendererBackend;
class ShaderProgram;
using UniqueShaderProgram = std::unique_ptr<ShaderProgram>;

class ShaderProgram final : public gfx::ShaderProgramBase {
public:
    ShaderProgram(std::string name,
                  RendererBackend& backend,
                  MTLFunctionPtr vertexFunction,
                  MTLFunctionPtr fragmentFunction);
    ~ShaderProgram() noexcept override = default;

    static constexpr std::string_view Name{"GenericMTLShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    MTLRenderPipelineStatePtr getRenderPipelineState(const gfx::RenderPassDescriptor&,
                                                     const MTLVertexDescriptorPtr&,
                                                     const gfx::ColorMode& colorMode) const;

    std::optional<uint32_t> getSamplerLocation(std::string_view name) const override;

    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }
    gfx::VertexAttributeArray& mutableVertexAttributes() override { return vertexAttributes; }

    const gfx::UniformBlockArray& getUniformBlocks() const override { return uniformBlocks; }
    gfx::UniformBlockArray& mutableUniformBlocks() override { return uniformBlocks; }

    void initAttribute(const shaders::AttributeInfo&);
    void initUniformBlock(const shaders::UniformBlockInfo&);
    void initTexture(const shaders::TextureInfo&);

protected:
    std::string shaderName;
    RendererBackend& backend;
    MTLFunctionPtr vertexFunction;
    MTLFunctionPtr fragmentFunction;
    UniformBlockArray uniformBlocks;
    VertexAttributeArray vertexAttributes;
    std::vector<std::string> textureBindings;
};

} // namespace mtl
} // namespace mbgl
