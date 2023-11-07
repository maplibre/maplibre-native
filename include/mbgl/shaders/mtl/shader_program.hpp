#pragma once

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>
#include <mbgl/mtl/uniform_block.hpp>
#include <mbgl/mtl/vertex_attribute.hpp>
#include <mbgl/shaders/shader_source.hpp>

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLLibrary.hpp>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace mbgl {
namespace shaders {
struct AttributeInfo {
    AttributeInfo(std::size_t index, gfx::AttributeDataType dataType, std::size_t count, std::string_view name);
    std::size_t index;
    gfx::AttributeDataType dataType;
    std::size_t count;
    std::string_view name;
    StringIdentity nameID;
};
struct UniformBlockInfo {
    UniformBlockInfo(std::size_t index, bool vertex, bool fragment, std::size_t size, std::string_view name);
    std::size_t index;
    bool vertex;
    bool fragment;
    std::size_t size;
    std::string_view name;
    StringIdentity nameID;
};
struct TextureInfo {
    TextureInfo(std::size_t index, std::string_view name);
    std::size_t index;
    std::string_view name;
    StringIdentity nameID;
};
struct ReflectionData {
    const std::string name;
    const std::string vertexMainFunction;
    const std::string fragmentMainFunction;
    const std::vector<const AttributeInfo> attributes;
    const std::vector<const UniformBlockInfo> uniforms;
    const std::vector<const TextureInfo> textures;
    // A link is a Metal shader compiled separate and linked later as a shader library
    const std::vector<shaders::BuiltIn> links;
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

    MTLRenderPipelineStatePtr getRenderPipelineState(const gfx::Renderable&,
                                                     const MTLVertexDescriptorPtr&,
                                                     const gfx::ColorMode& colorMode) const;

    std::optional<uint32_t> getSamplerLocation(const StringIdentity id) const override;

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
    std::unordered_map<StringIdentity, std::size_t> textureBindings;
};

} // namespace mtl
} // namespace mbgl
