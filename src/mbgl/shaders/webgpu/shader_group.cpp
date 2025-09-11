#include <mbgl/shaders/webgpu/shader_group.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

void ShaderGroup::initialize(Context& context) {
    // For now, create a simple test shader that can render basic geometry
    // In a complete implementation, all shader types would be registered here
    
    // Vertex shader that transforms tile coordinates using projection matrix
    const std::string vertexSource = R"(
struct Uniforms {
    mvp_matrix: mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

struct VertexInput {
    @location(0) position: vec2<i32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
}

@vertex
fn main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    
    // Convert int16 position to tile coordinates
    // MapLibre uses 0-8192 for tile coordinates
    let pos = vec2<f32>(f32(input.position.x), f32(input.position.y));
    
    // Apply the MVP matrix transformation
    output.position = uniforms.mvp_matrix * vec4<f32>(pos, 0.0, 1.0);
    
    // Use different colors based on position for debugging
    output.color = vec4<f32>(
        pos.x / 8192.0,
        pos.y / 8192.0,
        0.5,
        1.0
    );
    
    return output;
}
)";

    // Fragment shader that outputs the interpolated color
    const std::string fragmentSource = R"(
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
}

@fragment
fn main(input: VertexOutput) -> @location(0) vec4<f32> {
    return input.color;
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
    
    Log::Info(Event::General, "WebGPU shader group initialized with default shader");
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
    
    Log::Info(Event::General, "ShaderGroup::getOrCreateShader returning default shader");
    return defaultShader;
}

bool ShaderGroup::isShader(const std::string& shaderName) const noexcept {
    return shaders.find(shaderName) != shaders.end();
}

const gfx::ShaderPtr ShaderGroup::getShader(const std::string& shaderName) const noexcept {
    auto it = shaders.find(shaderName);
    if (it != shaders.end()) {
        Log::Info(Event::General, "ShaderGroup::getShader found shader: " + shaderName);
        return it->second;
    }
    Log::Info(Event::General, "ShaderGroup::getShader shader not found: " + shaderName);
    return nullptr;
}

} // namespace webgpu
} // namespace mbgl