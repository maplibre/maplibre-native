#pragma once

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <mbgl/gfx/attribute.hpp>
#include <string>
#include <unordered_map>
#include <array>

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

namespace webgpu {

class Context;

class ShaderProgram final : public gfx::ShaderProgramBase {
public:
    // Static name member required by is_shader_v trait
    static constexpr std::string_view Name = "WebGPUShader";

    // Constructor that takes attribute array from shader definitions
    template<size_t N>
    ShaderProgram(Context& ctx,
                  const std::string& vertexSource,
                  const std::string& fragmentSource,
                  const std::array<shaders::AttributeInfo, N>& attrs)
        : context(ctx) {
        // Store attributes for pipeline creation
        attributeInfos.reserve(N);
        for (const auto& attr : attrs) {
            attributeInfos.push_back(attr);
        }
        createPipeline(vertexSource, fragmentSource);
    }

    // Basic constructor
    ShaderProgram(Context& context,
                  const std::string& vertexSource,
                  const std::string& fragmentSource);
    ~ShaderProgram() override;

    // gfx::Shader interface (required for is_shader_v trait)
    const std::string_view typeName() const noexcept override { return Name; }
    std::optional<size_t> getSamplerLocation(const size_t) const override { return std::nullopt; }
    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }
    const gfx::VertexAttributeArray& getInstanceAttributes() const override { return instanceAttributes; }

    WGPURenderPipeline getPipeline() const { return pipeline; }
    WGPUBindGroupLayout getBindGroupLayout() const { return bindGroupLayout; }

private:
    void createPipeline(const std::string& vertexSource, const std::string& fragmentSource);
    static WGPUVertexFormat getWGPUFormat(gfx::AttributeDataType type);

    Context& context;
    WGPURenderPipeline pipeline = nullptr;
    WGPUBindGroupLayout bindGroupLayout = nullptr;
    WGPUPipelineLayout pipelineLayout = nullptr;
    WGPUShaderModule vertexShaderModule = nullptr;
    WGPUShaderModule fragmentShaderModule = nullptr;

    gfx::VertexAttributeArray vertexAttributes;
    gfx::VertexAttributeArray instanceAttributes;
    std::vector<shaders::AttributeInfo> attributeInfos;
};

} // namespace webgpu
} // namespace mbgl