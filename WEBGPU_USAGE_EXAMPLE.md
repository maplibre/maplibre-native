# WebGPU Backend Usage Example

This document shows how to use the WebGPU backend in MapLibre Native.

## Building with WebGPU

```bash
# Configure with WebGPU enabled
cmake -B build -DMLN_WITH_WEBGPU=ON -DMLN_WEBGPU_BACKEND=dawn

# Build
cmake --build build
```

## Example Code

### 1. Initialize the WebGPU Backend

```cpp
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/style/style.hpp>

// Create the WebGPU renderer backend
auto backend = std::make_unique<mbgl::webgpu::RendererBackend>(
    mbgl::gfx::ContextMode::Unique
);

// Set up the surface (platform-specific)
// For GLFW:
backend->setSurface(glfwGetWGPUSurface(window));

// Initialize shaders
mbgl::gfx::ShaderRegistry shaderRegistry;
mbgl::ProgramParameters programParams;
backend->initShaders(shaderRegistry, programParams);

// Create a map
mbgl::MapOptions mapOptions;
mapOptions.withSize(mbgl::Size{1024, 768});

auto map = std::make_unique<mbgl::Map>(
    *backend,
    mapOptions,
    mbgl::ResourceOptions()
);
```

### 2. Creating and Drawing with WebGPU

```cpp
// Get the WebGPU context
auto& context = static_cast<mbgl::webgpu::Context&>(backend->getContext());

// Create a shader program
auto shader = std::make_shared<mbgl::webgpu::ShaderProgram>(
    context,
    "MyShader",
    vertexWGSL,   // WGSL vertex shader source
    fragmentWGSL  // WGSL fragment shader source
);

// Create a drawable using the builder pattern
auto drawableBuilder = context.createDrawableBuilder("MyDrawable");

// Configure the drawable
drawableBuilder->setShader(shader)
    ->setVertexData(std::move(vertexData), vertexCount)
    ->setIndexData(indexVector, std::move(segments))
    ->setTextures(textures)
    ->setUniformBuffers(uniformBuffers)
    ->setColorMode(mbgl::gfx::ColorMode::alphaBlended())
    ->setDepthType(mbgl::gfx::DepthMaskType::ReadWrite)
    ->setCullFaceMode(mbgl::gfx::CullFaceMode::backCCW());

// Build the drawable
auto drawable = drawableBuilder->build();

// Create a command encoder
auto commandEncoder = context.createCommandEncoder();

// Create a render pass
mbgl::gfx::RenderPassDescriptor passDesc;
passDesc.color = colorAttachment;
passDesc.clearColor = mbgl::Color{0.0f, 0.0f, 0.0f, 1.0f};

auto renderPass = commandEncoder->createRenderPass("MainPass", passDesc);

// Draw the drawable
renderPass->draw(drawable);

// Present the frame
commandEncoder->present();
```

### 3. WGSL Shader Example

```wgsl
// Vertex shader
struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) texCoord: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) texCoord: vec2<f32>,
};

@group(0) @binding(0)
var<uniform> mvpMatrix: mat4x4<f32>;

@vertex
fn main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    output.position = mvpMatrix * vec4<f32>(input.position, 0.0, 1.0);
    output.texCoord = input.texCoord;
    return output;
}

// Fragment shader
@group(0) @binding(1)
var baseTexture: texture_2d<f32>;

@group(0) @binding(2)
var baseSampler: sampler;

@fragment
fn main(input: VertexOutput) -> @location(0) vec4<f32> {
    return textureSample(baseTexture, baseSampler, input.texCoord);
}
```

### 4. Creating Textures

```cpp
// Create a texture
auto texture = context.createTexture2D();

// Upload texture data
texture->upload(imageData, mbgl::Size{256, 256}, 
                mbgl::gfx::TextureChannelDataType::UnsignedByte);

// Configure sampling
texture->setSamplerConfiguration(
    mbgl::gfx::TextureFilterType::Linear,
    mbgl::gfx::TextureMipMapType::No,
    mbgl::gfx::TextureWrapType::Clamp
);
```

### 5. Creating Uniform Buffers

```cpp
// Define uniform data structure
struct MyUniforms {
    glm::mat4 mvpMatrix;
    glm::vec4 color;
};

MyUniforms uniforms;
uniforms.mvpMatrix = calculateMVP();
uniforms.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

// Create uniform buffer
auto uniformBuffer = context.createUniformBuffer(
    &uniforms, 
    sizeof(MyUniforms),
    false  // not persistent
);

// Update uniform buffer
uniformBuffer->update(&uniforms, sizeof(MyUniforms));
```

## Platform Integration

### GLFW (Desktop)

```cpp
// Initialize GLFW with WebGPU support
glfwInit();
glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
GLFWwindow* window = glfwCreateWindow(1024, 768, "MapLibre WebGPU", nullptr, nullptr);

// Get the WebGPU surface
WGPUSurface surface = glfwGetWGPUSurface(instance, window);
backend->setSurface(surface);
```

### Android

```cpp
// In your native activity
ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
backend->setSurface(window);
```

### iOS

```objc
// In your view controller
CAMetalLayer* metalLayer = [CAMetalLayer layer];
[view.layer addSublayer:metalLayer];
backend->setSurface((__bridge void*)metalLayer);
```

## Performance Considerations

1. **Pipeline Caching**: The WebGPU backend automatically caches render pipelines based on render state
2. **Buffer Updates**: Use persistent buffers for frequently updated data
3. **Texture Formats**: Use appropriate texture formats for your data (RGBA8 for colors, R8 for masks)
4. **Draw Calls**: Batch similar draw calls together to minimize state changes

## Debugging

Enable WebGPU validation layers in debug builds:

```cpp
// In backend_impl.cpp
WGPUInstanceDescriptor instanceDesc = {};
instanceDesc.backendType = WGPUBackendType_Vulkan; // or D3D12, Metal
#ifdef DEBUG
instanceDesc.enableValidation = true;
#endif
```

## Migration from OpenGL

Key differences when migrating from OpenGL to WebGPU:

1. **Shaders**: Convert GLSL to WGSL
2. **Uniforms**: Use uniform buffers instead of individual uniforms
3. **State**: Pipeline state is immutable in WebGPU
4. **Coordinates**: WebGPU uses a different coordinate system (Y-up in NDC)

## Limitations

- Line width is not directly supported (must be handled in shaders)
- No immediate mode rendering (all drawing is command-based)
- Requires modern GPU with Vulkan, D3D12, or Metal support

## Further Reading

- [WebGPU Specification](https://www.w3.org/TR/webgpu/)
- [WGSL Specification](https://www.w3.org/TR/WGSL/)
- [Dawn Documentation](https://dawn.googlesource.com/dawn)
- [wgpu Documentation](https://wgpu.rs/)