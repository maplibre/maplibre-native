#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "ClippingMaskProgram";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(2) position: vec2<i32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
}

struct PushConstants {
    matrix: mat4x4<f32>,
};

var<push_constant> constants: PushConstants;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = constants.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    return out;
}
)";
    
    static constexpr auto fragment = R"(
@fragment
fn main() -> @location(0) vec4<f32> {
    return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}
)";
};

} // namespace shaders
} // namespace mbgl