#pragma once

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>
#include <mbgl/mtl/vertex_attribute.hpp>

#include <Foundation/NSSharedPtr.hpp>

#include <optional>
#include <string>
#include <unordered_map>

namespace mbgl {
namespace shaders {

#ifndef MBGL_SHADERS_PROGRAM_COMMON_TYPES_DEFINED
#define MBGL_SHADERS_PROGRAM_COMMON_TYPES_DEFINED
struct UniformBlockInfo {
    constexpr UniformBlockInfo(bool vertex_, bool fragment_, std::size_t size_, std::size_t id_)
        : index(id_),
          vertex(vertex_),
          fragment(fragment_),
          size(size_),
          id(id_) {}
    std::size_t index;
    bool vertex;
    bool fragment;
    std::size_t size;
    std::size_t id;
};
struct AttributeInfo {
    constexpr AttributeInfo(std::size_t index_, gfx::AttributeDataType dataType_, std::size_t id_)
        : index(index_),
          dataType(dataType_),
          id(id_) {}
    std::size_t index;
    gfx::AttributeDataType dataType;
    std::size_t id;
};
struct TextureInfo {
    constexpr TextureInfo(std::size_t index_, std::size_t id_)
        : index(index_),
          id(id_) {}
    std::size_t index;
    std::size_t id;
};
#endif // MBGL_SHADERS_PROGRAM_COMMON_TYPES_DEFINED
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
    ~ShaderProgram() noexcept override;

    static constexpr std::string_view Name{"GenericMTLShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    MTLRenderPipelineStatePtr getRenderPipelineState(const gfx::Renderable&,
                                                     const MTLVertexDescriptorPtr&,
                                                     const gfx::ColorMode& colorMode,
                                                     const std::optional<std::size_t> reuseHash) const;

    std::optional<size_t> getSamplerLocation(const size_t id) const override;

    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }

    const gfx::VertexAttributeArray& getInstanceAttributes() const override { return instanceAttributes; }

    void initAttribute(const shaders::AttributeInfo&);
    void initInstanceAttribute(const shaders::AttributeInfo&);
    void initTexture(const shaders::TextureInfo&);

protected:
    std::string shaderName;
    RendererBackend& backend;
    MTLFunctionPtr vertexFunction;
    MTLFunctionPtr fragmentFunction;
    VertexAttributeArray vertexAttributes;
    VertexAttributeArray instanceAttributes;
    std::array<std::optional<size_t>, shaders::maxTextureCountPerShader> textureBindings;

    mutable mbgl::unordered_map<std::size_t, MTLRenderPipelineStatePtr> renderPipelineStateCache;
};

} // namespace mtl
} // namespace mbgl
