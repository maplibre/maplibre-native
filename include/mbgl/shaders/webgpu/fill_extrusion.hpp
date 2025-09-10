#pragma once

#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <array>
#include <string>

namespace mbgl {
namespace shaders {

class FillExtrusionShader final : public gfx::ShaderProgramBase {
public:
    FillExtrusionShader();
    ~FillExtrusionShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 4> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_normal_ed", gfx::AttributeType::Int4},
        {"a_base", gfx::AttributeType::Float},
        {"a_height", gfx::AttributeType::Float}
    }};
    
    static constexpr std::array<TextureInfo, 0> textures = {};
};

class FillExtrusionPatternShader final : public gfx::ShaderProgramBase {
public:
    FillExtrusionPatternShader();
    ~FillExtrusionPatternShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 5> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_normal_ed", gfx::AttributeType::Int4},
        {"a_base", gfx::AttributeType::Float},
        {"a_height", gfx::AttributeType::Float},
        {"a_pattern", gfx::AttributeType::UShort4}
    }};
    
    static constexpr std::array<TextureInfo, 1> textures = {{
        {"u_image", 0}
    }};
};

} // namespace shaders
} // namespace mbgl