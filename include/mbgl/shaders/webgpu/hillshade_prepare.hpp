#pragma once

#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <array>
#include <string>

namespace mbgl {
namespace shaders {

class HillshadePrepareShader final : public gfx::ShaderProgramBase {
public:
    HillshadePrepareShader();
    ~HillshadePrepareShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 2> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_texture_pos", gfx::AttributeType::Float2}
    }};
    
    static constexpr std::array<TextureInfo, 1> textures = {{
        {"u_image", 0}
    }};
};

} // namespace shaders
} // namespace mbgl