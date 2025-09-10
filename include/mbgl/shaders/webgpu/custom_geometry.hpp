#pragma once

#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <array>
#include <string>

namespace mbgl {
namespace shaders {

class CustomGeometryShader final : public gfx::ShaderProgramBase {
public:
    CustomGeometryShader();
    ~CustomGeometryShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 1> attributes = {{
        {"a_pos", gfx::AttributeType::Float2}
    }};
    
    static constexpr std::array<TextureInfo, 0> textures = {};
};

} // namespace shaders
} // namespace mbgl