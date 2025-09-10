#pragma once

#include <mbgl/gfx/shader_program.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace mbgl {
namespace webgpu {

class Context;

class ShaderProgram : public gfx::ShaderProgramBase {
public:
    ShaderProgram(Context& context,
                  const std::string& name,
                  const std::string& vertexSource,
                  const std::string& fragmentSource);
    ~ShaderProgram() override;

    // WebGPU specific
    WGPUShaderModule getVertexModule() const { return vertexModule; }
    WGPUShaderModule getFragmentModule() const { return fragmentModule; }
    WGPUPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    
    // Get or create a render pipeline for specific state
    WGPURenderPipeline getOrCreatePipeline(const gfx::ColorMode& colorMode,
                                          const gfx::CullFaceMode& cullFaceMode,
                                          const gfx::DepthMode& depthMode,
                                          const gfx::StencilMode& stencilMode,
                                          const gfx::VertexAttributeArray& attributes);

    // Uniform and binding management
    struct BindingInfo {
        uint32_t group;
        uint32_t binding;
        WGPUBufferBindingType type;
        uint64_t minBindingSize;
    };
    
    const std::unordered_map<std::string, BindingInfo>& getUniformBindings() const { return uniformBindings; }
    const std::unordered_map<std::string, BindingInfo>& getTextureBindings() const { return textureBindings; }

private:
    void compileShader(const std::string& vertexSource, const std::string& fragmentSource);
    void createPipelineLayout();
    void parseBindings(const std::string& source);
    
    Context& context;
    WGPUShaderModule vertexModule = nullptr;
    WGPUShaderModule fragmentModule = nullptr;
    WGPUPipelineLayout pipelineLayout = nullptr;
    
    // Cache of pipelines for different render states
    struct PipelineKey {
        gfx::ColorMode colorMode;
        gfx::CullFaceMode cullFaceMode;
        gfx::DepthMode depthMode;
        gfx::StencilMode stencilMode;
        size_t vertexLayoutHash;
        
        bool operator==(const PipelineKey& other) const;
    };
    
    struct PipelineKeyHash {
        size_t operator()(const PipelineKey& key) const;
    };
    
    std::unordered_map<PipelineKey, WGPURenderPipeline, PipelineKeyHash> pipelineCache;
    
    // Binding information
    std::unordered_map<std::string, BindingInfo> uniformBindings;
    std::unordered_map<std::string, BindingInfo> textureBindings;
    std::vector<WGPUBindGroupLayout> bindGroupLayouts;
};

} // namespace webgpu
} // namespace mbgl