#include <mbgl/shaders/webgpu/shader_group.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

void ShaderGroup::initialize(Context& context) {
    // For now, create a simple test shader that can render basic geometry
    // In a complete implementation, all shader types would be registered here

    // Vertex shader for line rendering with proper extrusion
    const std::string vertexSource = R"(
struct Uniforms {
    mvp_matrix: mat4x4<f32>,
    ratio: f32,  // Screen aspect ratio for line extrusion
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

struct VertexInput {
    @location(0) pos_normal: vec2<i32>,  // Packed position and normal data (int16x2)
    @location(1) data: vec4<u32>,        // Extrusion and direction data (uint8x4)
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) normal: vec2<f32>,
    @location(2) width: vec2<f32>,  // (outset, inset) for fragment shader
    @location(3) gamma_scale: f32,
}

// Constants for line rendering
const LINE_NORMAL_SCALE = 1.0 / 63.0;  // Scale factor for line normals
const DEVICE_PIXEL_RATIO = 1.0;
const ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

@vertex
fn main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;

    // Unpack position and normal from the packed vertex attribute
    let pos_normal_float = vec2<f32>(f32(input.pos_normal.x), f32(input.pos_normal.y));

    // Extract position by dividing by 2 and flooring
    let pos = floor(pos_normal_float * 0.5);

    // Extract normal from the remainder
    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    let normal = pos_normal_float - 2.0 * pos;
    let v_normal = vec2<f32>(normal.x, normal.y * 2.0 - 1.0);

    // Extract extrusion data from the data attribute
    // data.xy contains the extrusion vector offset by 128
    let a_extrude = vec2<f32>(f32(input.data.x), f32(input.data.y)) - 128.0;

    // Extract direction from data.z (mod 4) - 1
    let a_direction = (f32(input.data.z) % 4.0) - 1.0;

    // Line width parameters (simplified for now)
    let width = 2.0;  // Default line width
    let gapwidth = 0.0;
    let offset = 0.0;

    // Calculate line geometry parameters
    let halfwidth = width / 2.0;
    let inset = gapwidth + select(0.0, ANTIALIASING, gapwidth > 0.0);
    let outset = gapwidth + halfwidth * select(1.0, 2.0, gapwidth > 0.0) + select(0.0, ANTIALIASING, halfwidth != 0.0);

    // Scale the extrusion vector down to a normal and then up by the line width
    let dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line
    let u = 0.5 * a_direction;
    let t = 1.0 - abs(u);
    let rotation_matrix = mat2x2<f32>(t, -u, u, t);
    let offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * rotation_matrix;

    // Use default ratio if not provided (aspect ratio correction)
    let ratio = 1.0;  // This should come from uniforms in a complete implementation

    // Calculate final position with extrusion
    let projected_extrude = uniforms.mvp_matrix * vec4<f32>(dist / ratio, 0.0, 0.0);
    let position = uniforms.mvp_matrix * vec4<f32>(pos + offset2 / ratio, 0.0, 1.0) + projected_extrude;

    // Calculate gamma scale for antialiasing
    let extrude_length_without_perspective = length(dist);
    let units_to_pixels = 1.0;  // This should be calculated from the projection
    let extrude_length_with_perspective = length(projected_extrude.xy / position.w * units_to_pixels);
    let gamma_scale = extrude_length_without_perspective / max(extrude_length_with_perspective, 0.001);

    output.position = position;
    output.normal = v_normal;
    output.width = vec2<f32>(outset, inset);
    output.gamma_scale = gamma_scale;

    // Color based on position for debugging
    output.color = vec4<f32>(0.2, 0.5, 1.0, 1.0);  // Light blue color for lines

    return output;
}
)";

    // Fragment shader for line rendering with antialiasing
    const std::string fragmentSource = R"(
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) normal: vec2<f32>,
    @location(2) width: vec2<f32>,  // (outset, inset)
    @location(3) gamma_scale: f32,
}

const DEVICE_PIXEL_RATIO = 1.0;

@fragment
fn main(input: VertexOutput) -> @location(0) vec4<f32> {
    // Calculate the distance of the pixel from the line in pixels
    let dist = length(input.normal) * input.width.x;

    // Calculate the antialiasing fade factor
    // This is either when fading in the line in case of an offset line (width.y)
    // or when fading out (width.x)
    let blur = 0.5;  // Default blur value
    let blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * input.gamma_scale;
    let alpha = clamp(min(dist - (input.width.y - blur2), input.width.x - dist) / blur2, 0.0, 1.0);

    // Apply alpha to the color
    return vec4<f32>(input.color.rgb, input.color.a * alpha);
}
)";

    // Create a default shader program
    defaultShader = std::make_shared<ShaderProgram>(context, vertexSource, fragmentSource);

    // Register the default shader for all common shader names
    // This is a temporary solution - in production, each shader type would have its own implementation
    const std::vector<std::string> shaderNames = {
        "BackgroundShader",
        "BackgroundPatternShader",
        "CircleShader",
        "FillShader",
        "FillPatternShader",
        "FillExtrusionShader",
        "FillExtrusionPatternShader",
        "LineShader",
        "LinePatternShader",
        "LineSDFShader",
        "RasterShader",
        "HillshadeShader",
        "HillshadePrepareShader",
        "SymbolIconShader",
        "SymbolSDFIconShader",
        "SymbolTextAndIconShader"
    };

    for (const auto& name : shaderNames) {
        shaders[name] = defaultShader;
    }

}

gfx::ShaderPtr ShaderGroup::getOrCreateShader(gfx::Context& context,
                                              const StringIDSetsPair& propertiesAsUniforms,
                                              std::string_view firstAttribName) {
    // For now, return the default shader for all requests
    // In a complete implementation, this would generate shader variants
    // based on which properties are data-driven
    (void)context;
    (void)propertiesAsUniforms;
    (void)firstAttribName;

    return defaultShader;
}

bool ShaderGroup::isShader(const std::string& shaderName) const noexcept {
    return shaders.find(shaderName) != shaders.end();
}

const gfx::ShaderPtr ShaderGroup::getShader(const std::string& shaderName) const noexcept {
    auto it = shaders.find(shaderName);
    if (it != shaders.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace webgpu
} // namespace mbgl
