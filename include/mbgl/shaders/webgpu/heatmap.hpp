#pragma once

#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <array>
#include <string>

namespace mbgl {
namespace shaders {

class HeatmapShader final : public gfx::ShaderProgramBase {
public:
    HeatmapShader();
    ~HeatmapShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 3> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_weight", gfx::AttributeType::Float},
        {"a_radius", gfx::AttributeType::Float}
    }};
    
    static constexpr std::array<TextureInfo, 0> textures = {};
};

} // namespace shaders
} // namespace mbgl