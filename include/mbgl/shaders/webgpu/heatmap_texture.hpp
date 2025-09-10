#pragma once

#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <array>
#include <string>

namespace mbgl {
namespace shaders {

class HeatmapTextureShader final : public gfx::ShaderProgramBase {
public:
    HeatmapTextureShader();
    ~HeatmapTextureShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 2> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_texture_pos", gfx::AttributeType::Float2}
    }};
    
    static constexpr std::array<TextureInfo, 2> textures = {{
        {"u_image", 0},
        {"u_color_ramp", 1}
    }};
};

} // namespace shaders
} // namespace mbgl