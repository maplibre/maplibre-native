#pragma once

#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <array>
#include <string>

namespace mbgl {
namespace shaders {

class LineShader final : public gfx::ShaderProgramBase {
public:
    LineShader();
    ~LineShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 6> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_extrude", gfx::AttributeType::Float2},
        {"a_round_limit", gfx::AttributeType::Float},
        {"a_up_dir", gfx::AttributeType::Float},
        {"a_dir", gfx::AttributeType::Float},
        {"a_tile_distance_from_anchor", gfx::AttributeType::Float}
    }};
    
    static constexpr std::array<TextureInfo, 0> textures = {};
};

} // namespace shaders
} // namespace mbgl