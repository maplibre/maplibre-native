#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

#include <array>
#include <cstdint>

namespace mbgl {
namespace shaders {

struct alignas(16) ClipUBO {
    std::array<float, 4 * 4> matrix;
    std::uint32_t stencil_ref;
    float pad1;
    float pad2;
    float pad3;
};
static_assert(sizeof(ClipUBO) == 5 * 16, "wrong size");

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "ClippingMaskProgram";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(2) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
};

struct ClipUBO {
    matrix: mat4x4<f32>,
    stencil_ref: u32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(0) var<uniform> clip: ClipUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let clip_pos = clip.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    out.position = vec4<f32>(clip_pos.x, clip_pos.y, clip_pos.z, clip_pos.w);
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
