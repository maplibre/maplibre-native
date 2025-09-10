#pragma once

#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace webgpu {

class Context;

class ShaderGroup : public gfx::ShaderGroup {
public:
    ShaderGroup() = default;
    ~ShaderGroup() override = default;

    // Initialize all WebGPU shaders
    void initialize(Context& context);

    // Get or create a shader with data-driven properties
    gfx::ShaderPtr getOrCreateShader(gfx::Context& context,
                                     const StringIDSetsPair& propertiesAsUniforms,
                                     std::string_view firstAttribName = "a_pos") override;
};

} // namespace webgpu
} // namespace mbgl