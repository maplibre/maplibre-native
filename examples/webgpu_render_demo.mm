// WebGPU Rendering Demo - Renders a colored quad using Dawn
#include <iostream>
#include <vector>
#include <array>
#include <dawn/native/DawnNative.h>
#include <dawn/webgpu_cpp.h>
#include <GLFW/glfw3.h>

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#include <dawn/native/MetalBackend.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>
#endif

// Simple vertex shader in WGSL
const char* vertexShaderSource = R"(
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec3<f32>,
}

@vertex
fn main(@builtin(vertex_index) vertexIndex: u32) -> VertexOutput {
    var output: VertexOutput;
    
    // Define a quad with two triangles
    var positions = array<vec2<f32>, 6>(
        vec2<f32>(-0.5, -0.5),  // bottom left
        vec2<f32>( 0.5, -0.5),  // bottom right
        vec2<f32>( 0.5,  0.5),  // top right
        vec2<f32>(-0.5, -0.5),  // bottom left
        vec2<f32>( 0.5,  0.5),  // top right
        vec2<f32>(-0.5,  0.5)   // top left
    );
    
    // Colors for each vertex (creating a gradient)
    var colors = array<vec3<f32>, 6>(
        vec3<f32>(1.0, 0.0, 0.0),  // red
        vec3<f32>(0.0, 1.0, 0.0),  // green
        vec3<f32>(0.0, 0.0, 1.0),  // blue
        vec3<f32>(1.0, 0.0, 0.0),  // red
        vec3<f32>(0.0, 0.0, 1.0),  // blue
        vec3<f32>(1.0, 1.0, 0.0)   // yellow
    );
    
    output.position = vec4<f32>(positions[vertexIndex], 0.0, 1.0);
    output.color = colors[vertexIndex];
    return output;
}
)";

// Simple fragment shader in WGSL
const char* fragmentShaderSource = R"(
@fragment
fn main(@location(0) color: vec3<f32>) -> @location(0) vec4<f32> {
    return vec4<f32>(color, 1.0);
}
)";

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    // Create window without default API
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "WebGPU Render Demo - Colored Quad", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    
    std::cout << "✓ Created GLFW window\n";
    
    // Create Dawn instance
    auto instance = std::make_unique<dawn::native::Instance>();
    std::cout << "✓ Created Dawn instance\n";
    
    // Get adapters
    std::vector<dawn::native::Adapter> adapters = instance->EnumerateAdapters();
    std::cout << "Found " << adapters.size() << " adapter(s)\n";
    
    if (adapters.empty()) {
        std::cerr << "No adapters found\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // Use first adapter
    dawn::native::Adapter& adapter = adapters[0];
    std::cout << "✓ Selected adapter\n";
    
    // Create device
    WGPUDevice wgpuDevice = adapter.CreateDevice();
    if (!wgpuDevice) {
        std::cerr << "Failed to create device\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    wgpu::Device device = wgpu::Device::Acquire(wgpuDevice);
    wgpu::Queue queue = device.GetQueue();
    std::cout << "✓ Created WebGPU device\n";
    
    // Setup surface for macOS
    wgpu::Surface surface;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
#ifdef __APPLE__
    NSWindow* nsWindow = glfwGetCocoaWindow(window);
    NSView* view = [nsWindow contentView];
    [view setWantsLayer:YES];
    
    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.device = dawn::native::metal::GetMTLDevice(wgpuDevice);
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.drawableSize = CGSizeMake(width, height);
    [view setLayer:metalLayer];
    
    // Create surface
    wgpu::SurfaceDescriptorFromMetalLayer metalDesc;
    metalDesc.layer = (__bridge void*)metalLayer;
    
    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &metalDesc;
    
    auto dawnSurface = wgpu::Instance(instance->Get()).CreateSurface(&surfaceDesc);
    surface = dawnSurface;
#endif
    
    // Configure surface
    wgpu::SurfaceConfiguration surfaceConfig = {};
    surfaceConfig.device = device;
    surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
    surfaceConfig.format = wgpu::TextureFormat::BGRA8Unorm;
    surfaceConfig.width = width;
    surfaceConfig.height = height;
    surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
    surfaceConfig.alphaMode = wgpu::CompositeAlphaMode::Opaque;
    
    surface.Configure(&surfaceConfig);
    std::cout << "✓ Configured surface\n";
    
    // Create shaders
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    
    // Vertex shader
    wgslDesc.code = vertexShaderSource;
    wgpu::ShaderModuleDescriptor vertexShaderDesc;
    vertexShaderDesc.nextInChain = &wgslDesc;
    wgpu::ShaderModule vertexShader = device.CreateShaderModule(&vertexShaderDesc);
    
    // Fragment shader
    wgslDesc.code = fragmentShaderSource;
    wgpu::ShaderModuleDescriptor fragmentShaderDesc;
    fragmentShaderDesc.nextInChain = &wgslDesc;
    wgpu::ShaderModule fragmentShader = device.CreateShaderModule(&fragmentShaderDesc);
    
    std::cout << "✓ Created shaders\n";
    
    // Create render pipeline
    wgpu::ColorTargetState colorTarget = {};
    colorTarget.format = wgpu::TextureFormat::BGRA8Unorm;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;
    
    wgpu::FragmentState fragmentState = {};
    fragmentState.module = fragmentShader;
    fragmentState.entryPoint = "main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    
    wgpu::RenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.vertex.module = vertexShader;
    pipelineDesc.vertex.entryPoint = "main";
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDesc.multisample.count = 1;
    
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);
    std::cout << "✓ Created render pipeline\n";
    
    std::cout << "\n🎨 Rendering a colored quad. Press ESC or close window to exit.\n";
    
    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Check for ESC key
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }
        
        // Get next texture from surface
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        
        if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal) {
            std::cerr << "Failed to get surface texture\n";
            continue;
        }
        
        wgpu::TextureView textureView = surfaceTexture.texture.CreateView();
        
        // Create command encoder
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        
        // Create render pass
        wgpu::RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = textureView;
        colorAttachment.loadOp = wgpu::LoadOp::Clear;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        colorAttachment.clearValue = {0.1f, 0.2f, 0.3f, 1.0f};  // Dark blue background
        
        wgpu::RenderPassDescriptor renderPassDesc = {};
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        
        // Draw our quad
        pass.SetPipeline(pipeline);
        pass.Draw(6);  // 6 vertices for 2 triangles
        
        pass.End();
        
        // Submit commands
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        
        // Present
        surface.Present();
    }
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "✓ Clean shutdown\n";
    return 0;
}