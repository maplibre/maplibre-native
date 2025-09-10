#pragma once

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <string>
#include <unordered_map>

namespace mbgl {
namespace webgpu {

class Context;

class ShaderProgram : public gfx::ShaderProgramBase {
public:
    ShaderProgram(Context& context,
                  const std::string& vertexSource,
                  const std::string& fragmentSource);
    ~ShaderProgram() override;

    // ShaderProgramBase interface
    const std::string_view typeName() const noexcept override { return "WebGPU"; }
    std::optional<size_t> getSamplerLocation(const size_t) const override { return std::nullopt; }
    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }
    const gfx::VertexAttributeArray& getInstanceAttributes() const override { return instanceAttributes; }

    WGPURenderPipeline getPipeline() const { return pipeline; }
    WGPUBindGroupLayout getBindGroupLayout() const { return bindGroupLayout; }

private:
    void createPipeline(const std::string& vertexSource, const std::string& fragmentSource);

    Context& context;
    WGPURenderPipeline pipeline = nullptr;
    WGPUBindGroupLayout bindGroupLayout = nullptr;
    WGPUPipelineLayout pipelineLayout = nullptr;
    WGPUShaderModule vertexShaderModule = nullptr;
    WGPUShaderModule fragmentShaderModule = nullptr;
    
    gfx::VertexAttributeArray vertexAttributes;
    gfx::VertexAttributeArray instanceAttributes;
};

} // namespace webgpu
} // namespace mbgl