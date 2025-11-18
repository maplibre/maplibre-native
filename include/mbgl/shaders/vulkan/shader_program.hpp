#pragma once

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/vulkan/vertex_attribute.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/pipeline.hpp>

#include <optional>
#include <string>
#include <unordered_map>

namespace mbgl {
namespace shaders {
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
} // namespace shaders

namespace vulkan {
class RenderableResource;
class RendererBackend;
class Context;
class ShaderProgram;
using UniqueShaderProgram = std::unique_ptr<ShaderProgram>;

class ShaderProgram final : public gfx::ShaderProgramBase {
public:
    ShaderProgram(shaders::BuiltIn shaderID,
                  const std::string& name,
                  const std::string_view& vertex,
                  const std::string_view& fragment,
                  const ProgramParameters& programParameters,
                  const mbgl::unordered_map<std::string, std::string>& additionalDefines,
                  RendererBackend& backend,
                  gfx::ContextObserver& observer);
    ~ShaderProgram() noexcept override;

    static constexpr std::string_view Name{"GenericVulkanShader"};
    const std::string_view typeName() const noexcept override { return Name; }

    const vk::UniquePipeline& getPipeline(const PipelineInfo& pipelineInfo);

    std::optional<size_t> getSamplerLocation(const size_t id) const override { return textureBindings[id]; }
    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }
    const gfx::VertexAttributeArray& getInstanceAttributes() const override { return instanceAttributes; }

    bool hasTextures() const;

    void initAttribute(const shaders::AttributeInfo&);
    void initInstanceAttribute(const shaders::AttributeInfo&);
    void initTexture(const shaders::TextureInfo&);

protected:
    std::string shaderName;
    RendererBackend& backend;
    Context& context;

    vk::UniqueShaderModule vertexShader;
    vk::UniqueShaderModule fragmentShader;
    std::shared_ptr<std::unordered_map<std::size_t, vk::UniquePipeline>> pipelines;

    VertexAttributeArray vertexAttributes;
    VertexAttributeArray instanceAttributes;
    std::array<std::optional<size_t>, shaders::maxTextureCountPerShader> textureBindings;
};

} // namespace vulkan
} // namespace mbgl
