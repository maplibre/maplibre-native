#pragma once

#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <array>
#include <string>

namespace mbgl {
namespace shaders {

class WideVectorShader final : public gfx::ShaderProgramBase {
public:
    WideVectorShader();
    ~WideVectorShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 10> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_data", gfx::AttributeType::UByte4},
        {"a_uv", gfx::AttributeType::Float2},
        {"a_fade_opacity", gfx::AttributeType::Float},
        {"a_opacity", gfx::AttributeType::Float},
        {"a_placed", gfx::AttributeType::UByte2},
        {"a_hidden", gfx::AttributeType::UByte2},
        {"a_projected_pos", gfx::AttributeType::Float3},
        {"a_instance", gfx::AttributeType::Float},
        {"a_index", gfx::AttributeType::Float}
    }};
    
    static constexpr std::array<TextureInfo, 1> textures = {{
        {"u_texture", 0}
    }};
};

} // namespace shaders
} // namespace mbgl