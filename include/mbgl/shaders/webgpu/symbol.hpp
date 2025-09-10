#pragma once

#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/layer_ubo.hpp>

#include <array>
#include <string>

namespace mbgl {
namespace shaders {

class SymbolIconShader final : public gfx::ShaderProgramBase {
public:
    SymbolIconShader();
    ~SymbolIconShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 4> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_offset", gfx::AttributeType::Float2},
        {"a_texture_pos", gfx::AttributeType::UShort4},
        {"a_data", gfx::AttributeType::UByte4}
    }};
    
    static constexpr std::array<TextureInfo, 1> textures = {{
        {"u_texture", 0}
    }};
};

class SymbolSDFShader final : public gfx::ShaderProgramBase {
public:
    SymbolSDFShader();
    ~SymbolSDFShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 7> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_offset", gfx::AttributeType::Float2},
        {"a_texture_pos", gfx::AttributeType::UShort4},
        {"a_data", gfx::AttributeType::UByte4},
        {"a_pixeloffset", gfx::AttributeType::Float4},
        {"a_minzoom", gfx::AttributeType::Float},
        {"a_maxzoom", gfx::AttributeType::Float}
    }};
    
    static constexpr std::array<TextureInfo, 1> textures = {{
        {"u_texture", 0}
    }};
};

class SymbolTextAndIconShader final : public gfx::ShaderProgramBase {
public:
    SymbolTextAndIconShader();
    ~SymbolTextAndIconShader() override;

    const std::string programIdentifier;

    static constexpr std::array<AttributeInfo, 7> attributes = {{
        {"a_pos", gfx::AttributeType::Float2},
        {"a_offset", gfx::AttributeType::Float2},
        {"a_texture_pos", gfx::AttributeType::UShort4},
        {"a_data", gfx::AttributeType::UByte4},
        {"a_pixeloffset", gfx::AttributeType::Float4},
        {"a_minzoom", gfx::AttributeType::Float},
        {"a_maxzoom", gfx::AttributeType::Float}
    }};
    
    static constexpr std::array<TextureInfo, 2> textures = {{
        {"u_texture", 0},
        {"u_texture_icon", 1}
    }};
};

} // namespace shaders
} // namespace mbgl