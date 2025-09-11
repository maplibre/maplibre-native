// Dawn WebGPU + GLFW Triangle Example
// Renders a colored triangle using Dawn's Metal backend on macOS

#include <iostream>
#include <vector>
#include <cstring>

// Dawn headers
#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <dawn/native/MetalBackend.h>
#include <webgpu/webgpu_cpp.h>

// GLFW headers
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// Platform-specific headers for Metal
#ifdef __APPLE__
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>
#endif

// Triangle vertex data
const float triangleVertexData[] = {
    // positions        // colors
     0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top (red)
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left (green)
     0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // bottom right (blue)
};

// WGSL shader code
const char* shaderCode = R"(
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec3<f32>,
}

@vertex
fn vs_main(@location(0) position: vec3<f32>,
           @location(1) color: vec3<f32>) -> VertexOutput {
    var output: VertexOutput;
    output.position = vec4<f32>(position, 1.0);
    output.color = color;
    return output;
}

@fragment
fn fs_main(@location(0) color: vec3<f32>) -> @location(0) vec4<f32> {
    return vec4<f32>(color, 1.0);
}
)";

class WebGPUApp {
public:
    bool initialize(GLFWwindow* window) {
        this->window = window;
        
        std::cout << "Initializing Dawn WebGPU...\n";
        
        // Initialize Dawn
        dawnProcSetProcs(&dawn::native::GetProcs());
        
        // Create instance (validation is enabled by default in debug builds)
        // No need to explicitly discover adapters, EnumerateAdapters will do it
        
        // Get Metal adapter
        wgpu::RequestAdapterOptions adapterOptions = {};
        adapterOptions.backendType = wgpu::BackendType::Metal;
        
        std::vector<dawn::native::Adapter> adapters = dawnInstance.EnumerateAdapters(&adapterOptions);
        
        if (adapters.empty()) {
            std::cerr << "No Metal adapter found\n";
            return false;
        }
        
        adapter = wgpu::Adapter(adapters[0].Get());
        std::cout << "✓ Got Metal adapter\n";
        
        // Create surface
        if (!createSurface()) {
            return false;
        }
        
        // Create device
        if (!createDevice()) {
            return false;
        }
        
        // Configure surface
        if (!configureSurface()) {
            return false;
        }
        
        // Create pipeline
        if (!createPipeline()) {
            return false;
        }
        
        // Create vertex buffer
        if (!createVertexBuffer()) {
            return false;
        }
        
        std::cout << "✓ WebGPU initialization complete!\n";
        return true;
    }
    
    void render() {
        // Get next texture from surface
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        
        if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
            surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
            std::cerr << "Failed to get surface texture\n";
            return;
        }
        
        wgpu::TextureView textureView = surfaceTexture.texture.CreateView();
        if (!textureView) {
            std::cerr << "Failed to get swap chain texture\n";
            return;
        }
        
        // Create command encoder
        wgpu::CommandEncoderDescriptor encoderDesc = {};
        encoderDesc.label = "Command Encoder";
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);
        
        // Create render pass
        wgpu::RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = textureView;
        colorAttachment.loadOp = wgpu::LoadOp::Clear;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        colorAttachment.clearValue = {0.1, 0.1, 0.2, 1.0};  // Dark blue background
        
        wgpu::RenderPassDescriptor renderPassDesc = {};
        renderPassDesc.label = "Render Pass";
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);
        
        // Draw triangle
        renderPass.SetPipeline(pipeline);
        renderPass.SetVertexBuffer(0, vertexBuffer);
        renderPass.Draw(3);
        
        // End render pass
        renderPass.End();
        
        // Submit commands
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        
        // Present
        surface.Present();
    }
    
    void cleanup() {
        // WebGPU resources are automatically cleaned up by destructors
    }
    
private:
    GLFWwindow* window = nullptr;
    dawn::native::Instance dawnInstance;
    wgpu::Surface surface;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::Texture currentTexture;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    
    bool createSurface() {
#ifdef __APPLE__
        // Get the native window handle
        NSWindow* nsWindow = glfwGetCocoaWindow(window);
        if (!nsWindow) {
            std::cerr << "Failed to get NSWindow from GLFW\n";
            return false;
        }
        
        // Create CAMetalLayer
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        [nsWindow.contentView setWantsLayer:YES];
        [nsWindow.contentView setLayer:metalLayer];
        
        // Get window size for layer
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        [metalLayer setDrawableSize:CGSizeMake(width, height)];
        
        // Create surface descriptor
        wgpu::SurfaceDescriptorFromMetalLayer metalDesc = {};
        metalDesc.layer = (__bridge void*)metalLayer;
        
        wgpu::SurfaceDescriptor surfaceDesc = {};
        surfaceDesc.nextInChain = &metalDesc;
        surfaceDesc.label = "Metal Surface";
        
        // Create surface from Dawn instance
        wgpu::Instance instance = wgpu::Instance(dawnInstance.Get());
        surface = instance.CreateSurface(&surfaceDesc);
        
        if (!surface) {
            std::cerr << "Failed to create surface\n";
            return false;
        }
        
        std::cout << "✓ Created Metal surface\n";
        return true;
#else
        std::cerr << "Only macOS is supported in this example\n";
        return false;
#endif
    }
    
    bool createDevice() {
        // Request device
        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.label = "Dawn Device";
        
        // Set up required features
        std::vector<wgpu::FeatureName> requiredFeatures;
        deviceDesc.requiredFeatures = requiredFeatures.data();
        deviceDesc.requiredFeatureCount = requiredFeatures.size();
        
        // Create device synchronously (Dawn supports this)
        device = adapter.CreateDevice(&deviceDesc);
        if (!device) {
            std::cerr << "Failed to create device\n";
            return false;
        }
        
        // Set up error callback using the C++ wrapper
        device.SetLoggingCallback(
            [](wgpu::LoggingType type, char const * message) {
                if (type == wgpu::LoggingType::Error) {
                    std::cerr << "WebGPU Error: " << message << "\n";
                }
            });
        
        // Get queue
        queue = device.GetQueue();
        
        std::cout << "✓ Created device and queue\n";
        return true;
    }
    
    bool configureSurface() {
        // Get window size
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
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
        
        std::cout << "✓ Configured surface (" << width << "x" << height << ")\n";
        return true;
    }
    
    bool createPipeline() {
        // Create shader module
        wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
        wgslDesc.code = shaderCode;
        
        wgpu::ShaderModuleDescriptor shaderDesc = {};
        shaderDesc.nextInChain = &wgslDesc;
        shaderDesc.label = "Triangle Shader";
        
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderDesc);
        if (!shaderModule) {
            std::cerr << "Failed to create shader module\n";
            return false;
        }
        
        // Vertex attributes
        std::vector<wgpu::VertexAttribute> attributes(2);
        attributes[0].format = wgpu::VertexFormat::Float32x3;
        attributes[0].offset = 0;
        attributes[0].shaderLocation = 0;
        
        attributes[1].format = wgpu::VertexFormat::Float32x3;
        attributes[1].offset = 3 * sizeof(float);
        attributes[1].shaderLocation = 1;
        
        // Vertex buffer layout
        wgpu::VertexBufferLayout vertexLayout = {};
        vertexLayout.arrayStride = 6 * sizeof(float);
        vertexLayout.stepMode = wgpu::VertexStepMode::Vertex;
        vertexLayout.attributeCount = attributes.size();
        vertexLayout.attributes = attributes.data();
        
        // Pipeline layout (no bind groups for this simple example)
        wgpu::PipelineLayoutDescriptor layoutDesc = {};
        layoutDesc.bindGroupLayoutCount = 0;
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&layoutDesc);
        
        // Create pipeline
        wgpu::RenderPipelineDescriptor pipelineDesc = {};
        pipelineDesc.label = "Triangle Pipeline";
        pipelineDesc.layout = pipelineLayout;
        
        // Vertex stage
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &vertexLayout;
        
        // Fragment stage
        wgpu::FragmentState fragmentState = {};
        fragmentState.module = shaderModule;
        fragmentState.entryPoint = "fs_main";
        
        wgpu::ColorTargetState colorTarget = {};
        colorTarget.format = wgpu::TextureFormat::BGRA8Unorm;
        colorTarget.writeMask = wgpu::ColorWriteMask::All;
        
        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;
        
        pipelineDesc.fragment = &fragmentState;
        
        // Primitive state
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
        pipelineDesc.primitive.cullMode = wgpu::CullMode::None;
        
        // Multisample state
        pipelineDesc.multisample.count = 1;
        pipelineDesc.multisample.mask = 0xFFFFFFFF;
        
        pipeline = device.CreateRenderPipeline(&pipelineDesc);
        if (!pipeline) {
            std::cerr << "Failed to create pipeline\n";
            return false;
        }
        
        std::cout << "✓ Created render pipeline\n";
        return true;
    }
    
    bool createVertexBuffer() {
        wgpu::BufferDescriptor bufferDesc = {};
        bufferDesc.label = "Vertex Buffer";
        bufferDesc.size = sizeof(triangleVertexData);
        bufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        
        vertexBuffer = device.CreateBuffer(&bufferDesc);
        if (!vertexBuffer) {
            std::cerr << "Failed to create vertex buffer\n";
            return false;
        }
        
        // Upload vertex data
        queue.WriteBuffer(vertexBuffer, 0, triangleVertexData, sizeof(triangleVertexData));
        
        std::cout << "✓ Created vertex buffer\n";
        return true;
    }
};

int main() {
    std::cout << "Dawn WebGPU Triangle Example\n";
    std::cout << "============================\n\n";
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }
    
    // Configure GLFW for WebGPU (no default OpenGL context)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Dawn WebGPU Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }
    
    std::cout << "✓ Created GLFW window (800x600)\n";
    
    // Initialize WebGPU
    WebGPUApp app;
    if (!app.initialize(window)) {
        std::cerr << "Failed to initialize WebGPU\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    std::cout << "\n🎨 Rendering colored triangle with Dawn WebGPU (Metal backend)\n";
    std::cout << "Press ESC or close window to exit\n\n";
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Check for ESC key
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
        // Render
        app.render();
    }
    
    // Cleanup
    app.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "\n✓ Cleanup complete\n";
    return 0;
}