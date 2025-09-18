#pragma once

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <string>
#include <unordered_map>
#include <array>
#include <optional>

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

namespace webgpu {

class Context;
class RendererBackend;

class ShaderProgram final : public gfx::ShaderProgramBase {
public:
    // Static name member required by is_shader_v trait
    static constexpr std::string_view Name = "WebGPUShader";

    enum class BindingType {
        UniformBuffer,
        ReadOnlyStorageBuffer,
        StorageBuffer,
        Sampler,
        Texture
    };

    struct BindingInfo {
        uint32_t group = 0;
        uint32_t binding = 0;
        BindingType type = BindingType::UniformBuffer;
        WGPUShaderStage visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    };

    // Metal-like constructor: takes pre-compiled shader modules
    ShaderProgram(std::string name,
                  RendererBackend& backend,
                  WGPUShaderModule vertexModule,
                  WGPUShaderModule fragmentModule);

    // Minimal constructor for Context::createShader compatibility
    ShaderProgram(Context& context,
                  const std::string& vertexSource,
                  const std::string& fragmentSource);

    // Template constructor for shader_group compatibility (required by shader definitions)
    template<size_t N, size_t M>
    ShaderProgram(Context& context,
                  const std::string& vertexSource,
                  const std::string& fragmentSource,
                  const std::array<shaders::AttributeInfo, N>& attrs,
                  const std::array<shaders::TextureInfo, M>& texts,
                  const std::string& name = "ShaderProgram")
        : ShaderProgram(context, vertexSource, fragmentSource) {
        // Update the shader name if provided
        if (name != "ShaderProgram") {
            shaderName = name;
        }
        // Initialize attributes like Metal does
        for (const auto& attr : attrs) {
            initAttribute(attr);
        }
        // Initialize textures like Metal does
        for (const auto& tex : texts) {
            initTexture(tex);
        }
    }

    ~ShaderProgram() override;

    // Metal-like lazy pipeline creation with caching
    WGPURenderPipeline getRenderPipeline(const gfx::Renderable& renderable,
                                        const WGPUVertexBufferLayout* vertexLayouts,
                                        uint32_t vertexLayoutCount,
                                        const gfx::ColorMode& colorMode,
                                        const gfx::DepthMode& depthMode,
                                        const gfx::StencilMode& stencilMode,
                                        const std::optional<std::size_t> reuseHash = std::nullopt);

    const std::vector<BindingInfo>& getBindingInfos() const { return bindingInfos; }
    const std::vector<BindingInfo>& getBindingInfosForGroup(uint32_t group) const;
    WGPUBindGroupLayout getBindGroupLayout(uint32_t group) const;
    const std::vector<WGPUBindGroupLayout>& getBindGroupLayouts() const { return bindGroupLayouts; }
    const std::vector<uint32_t>& getBindGroupOrder() const { return bindGroupOrder; }

    // gfx::Shader interface (required for is_shader_v trait)
    const std::string_view typeName() const noexcept override { return Name; }
    std::optional<size_t> getSamplerLocation(const size_t id) const override;
    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }
    const gfx::VertexAttributeArray& getInstanceAttributes() const override { return instanceAttributes; }

    // Metal-like attribute/texture initialization
    void initAttribute(const shaders::AttributeInfo& info);
    void initInstanceAttribute(const shaders::AttributeInfo& info);
    void initTexture(const shaders::TextureInfo& info);


protected:
    void createPipelineLayout(const std::string& vertexSource, const std::string& fragmentSource);
    WGPURenderPipeline createPipeline(const WGPUVertexBufferLayout* vertexLayouts,
                                     uint32_t vertexLayoutCount,
                                     const gfx::ColorMode& colorMode,
                                     const gfx::DepthMode& depthMode,
                                     const gfx::StencilMode& stencilMode);

    void analyzeShaderBindings(const std::string& source, WGPUShaderStage stage);
    void rebuildBindGroupLayouts();

    std::string shaderName;
    RendererBackend& backend;

    // Metal-like resource management
    WGPUShaderModule vertexShaderModule = nullptr;
    WGPUShaderModule fragmentShaderModule = nullptr;
    std::vector<WGPUBindGroupLayout> bindGroupLayouts;
    std::vector<uint32_t> bindGroupOrder;
    WGPUPipelineLayout pipelineLayout = nullptr;

    // Metal-like pipeline caching
    mutable std::unordered_map<std::size_t, WGPURenderPipeline> renderPipelineCache;

    // Metal-like attribute/texture management
    gfx::VertexAttributeArray vertexAttributes;
    gfx::VertexAttributeArray instanceAttributes;
    std::array<std::optional<size_t>, shaders::maxTextureCountPerShader> textureBindings;

    std::vector<BindingInfo> bindingInfos;
    std::vector<std::vector<BindingInfo>> bindingsPerGroup;
};

} // namespace webgpu
} // namespace mbgl
