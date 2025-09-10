#pragma once

#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <array>
#include <string>

namespace mbgl {
namespace shaders {

class CollisionBoxShader final : public gfx::ShaderProgramBase {
public:
    CollisionBoxShader();
    ~CollisionBoxShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 3> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_extrude", gfx::AttributeType::Float2},
        {"a_data", gfx::AttributeType::UByte2}
    }};
    
    static constexpr std::array<TextureInfo, 0> textures = {};
};

class CollisionCircleShader final : public gfx::ShaderProgramBase {
public:
    CollisionCircleShader();
    ~CollisionCircleShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 3> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_extrude", gfx::AttributeType::Float2},
        {"a_placed", gfx::AttributeType::UByte2}
    }};
    
    static constexpr std::array<TextureInfo, 0> textures = {};
};

} // namespace shaders
} // namespace mbgl