#pragma once

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <string>
#include <unordered_map>
#include <array>
#include <optional>

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
class RendererBackend;

class ShaderProgram final : public gfx::ShaderProgramBase {
public:
    // Static name member required by is_shader_v trait
    static constexpr std::string_view Name = "WebGPUShader";

    // Metal-like constructor: takes pre-compiled shader modules
    ShaderProgram(std::string name,
                  RendererBackend& backend,
                  WGPUShaderModule vertexModule,
                  WGPUShaderModule fragmentModule);

    // Constructor that takes attribute array from shader definitions (for compatibility)
    template<size_t N>
    ShaderProgram(Context& ctx,
                  const std::string& vertexSource,
                  const std::string& fragmentSource,
                  const std::array<shaders::AttributeInfo, N>& attrs)
        : context(&ctx) {
        // Store attributes for pipeline creation
        attributeInfos.reserve(N);
        for (const auto& attr : attrs) {
            attributeInfos.push_back(attr);
        }
        createShaderModules(vertexSource, fragmentSource);
        createPipelineLayout();
    }

    // Basic constructor (for compatibility)
    ShaderProgram(Context& context,
                  const std::string& vertexSource,
                  const std::string& fragmentSource);
    ~ShaderProgram() override;

    // Metal-like lazy pipeline creation with caching
    WGPURenderPipeline getRenderPipeline(const gfx::Renderable& renderable,
                                        const WGPUVertexBufferLayout* vertexLayouts,
                                        uint32_t vertexLayoutCount,
                                        const gfx::ColorMode& colorMode,
                                        const std::optional<std::size_t> reuseHash = std::nullopt);

    // Overload without Renderable for drawable use
    WGPURenderPipeline getRenderPipeline(const WGPUVertexBufferLayout* vertexLayouts,
                                        uint32_t vertexLayoutCount);

    // gfx::Shader interface (required for is_shader_v trait)
    const std::string_view typeName() const noexcept override { return Name; }
    std::optional<size_t> getSamplerLocation(const size_t id) const override;
    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }
    const gfx::VertexAttributeArray& getInstanceAttributes() const override { return instanceAttributes; }

    // Metal-like attribute/texture initialization
    void initAttribute(const shaders::AttributeInfo& info);
    void initInstanceAttribute(const shaders::AttributeInfo& info);
    void initTexture(const shaders::TextureInfo& info);

    WGPURenderPipeline getPipeline() const { 
        // Return the most recently cached pipeline, or nullptr if none
        if (!renderPipelineCache.empty()) {
            return renderPipelineCache.begin()->second;
        }
        return nullptr; 
    }
    WGPUBindGroupLayout getBindGroupLayout() const { return bindGroupLayout; }
    const std::vector<shaders::AttributeInfo>& getAttributeInfos() const { return attributeInfos; }

    // Make getWGPUFormat public for use in drawable
    static WGPUVertexFormat getWGPUFormat(gfx::AttributeDataType type);

private:
    void createShaderModules(const std::string& vertexSource, const std::string& fragmentSource);
    void createPipelineLayout();
    WGPURenderPipeline createPipeline(const WGPUVertexBufferLayout* vertexLayouts,
                                     uint32_t vertexLayoutCount,
                                     const gfx::ColorMode& colorMode);
    static WGPUBlendOperation getWGPUBlendOperation(gfx::ColorBlendEquationType equation);
    static WGPUBlendFactor getWGPUBlendFactor(gfx::ColorBlendFactorType factor);

    std::string shaderName;
    RendererBackend* backend = nullptr;
    Context* context = nullptr;

    // Metal-like resource management
    WGPUShaderModule vertexShaderModule = nullptr;
    WGPUShaderModule fragmentShaderModule = nullptr;
    WGPUBindGroupLayout bindGroupLayout = nullptr;
    WGPUPipelineLayout pipelineLayout = nullptr;

    // Metal-like pipeline caching
    mutable std::unordered_map<std::size_t, WGPURenderPipeline> renderPipelineCache;

    // Metal-like attribute/texture management
    gfx::VertexAttributeArray vertexAttributes;
    gfx::VertexAttributeArray instanceAttributes;
    std::array<std::optional<size_t>, shaders::maxTextureCountPerShader> textureBindings;

    std::vector<shaders::AttributeInfo> attributeInfos;
};

} // namespace webgpu
} // namespace mbgl