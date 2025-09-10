#pragma once

#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <string>

namespace mbgl {
namespace webgpu {

class ShaderProgram : public gfx::ShaderProgramBase {
public:
    explicit ShaderProgram(const std::string& name);
    ~ShaderProgram() override;
    
    // ShaderProgramBase overrides
    std::optional<size_t> getSamplerLocation(const size_t) const override { return std::nullopt; }
    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }
    const gfx::VertexAttributeArray& getInstanceAttributes() const override { return instanceAttributes; }
    
    const std::string& getName() const { return name_; }
    void* getPipeline() const { return pipeline; }

private:
    std::string name_;
    void* pipeline = nullptr;
    void* vertexShader = nullptr;
    void* fragmentShader = nullptr;
    gfx::VertexAttributeArray vertexAttributes;
    gfx::VertexAttributeArray instanceAttributes;
};

} // namespace webgpu
} // namespace mbgl