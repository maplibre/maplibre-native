// Real WebGPU + Dawn + GLFW Triangle Demo
// This actually renders using Dawn!

#include <iostream>
#include <memory>
#include <vector>
#include <cstring>

// Dawn WebGPU headers
#include <webgpu/webgpu_cpp.h>
#include <dawn/native/DawnNative.h>
#include <dawn/native/MetalBackend.h>

// GLFW headers
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// Triangle vertex data
const float triangleVertices[] = {
    // positions        // colors
     0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top (red)
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left (green)
     0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // bottom right (blue)
};

// WGSL Shader
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

class DawnWebGPUApp {
public:
    bool initialize(GLFWwindow* window) {
        this->window = window;
        
        // Create Dawn instance
        instance = std::make_unique<dawn::native::Instance>();
        instance->DiscoverDefaultAdapters();
        
        // Get Metal adapter
        std::vector<dawn::native::Adapter> adapters = instance->GetAdapters();
        dawn::native::Adapter* metalAdapter = nullptr;
        
        for (auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);
            std::cout << "Found adapter: " << properties.name << "\n";
            if (properties.backendType == wgpu::BackendType::Metal) {
                metalAdapter = &adapter;
                break;
            }
        }
        
        if (!metalAdapter) {
            std::cerr << "No Metal adapter found\n";
            return false;
        }
        
        // Create device
        WGPUDevice cDevice = metalAdapter->CreateDevice();
        device = wgpu::Device::Acquire(cDevice);
        
        // Set up error callback
        device.SetUncapturedErrorCallback(
            [](WGPUErrorType type, const char* message, void*) {
                std::cerr << "WebGPU error: " << message << "\n";
            }, nullptr);
        
        // Create surface from GLFW window
        NSWindow* nsWindow = glfwGetCocoaWindow(window);
        NSView* view = [nsWindow contentView];
        [view setWantsLayer:YES];
        
        // Create Metal layer
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        [view setLayer:metalLayer];
        
        // Create Dawn surface
        wgpu::SurfaceDescriptorFromMetalLayer metalDesc;
        metalDesc.layer = metalLayer;
        
        wgpu::SurfaceDescriptor surfaceDesc;
        surfaceDesc.nextInChain = &metalDesc;
        
        surface = wgpu::Instance(instance->Get()).CreateSurface(&surfaceDesc);
        
        // Configure surface
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        wgpu::SwapChainDescriptor swapChainDesc = {};
        swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
        swapChainDesc.format = wgpu::TextureFormat::BGRA8Unorm;
        swapChainDesc.width = width;
        swapChainDesc.height = height;
        swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
        
        swapChain = device.CreateSwapChain(surface, &swapChainDesc);
        
        // Create shader module
        wgpu::ShaderModuleWGSLDescriptor wgslDesc;
        wgslDesc.code = shaderCode;
        
        wgpu::ShaderModuleDescriptor shaderDesc;
        shaderDesc.nextInChain = &wgslDesc;
        
        shaderModule = device.CreateShaderModule(&shaderDesc);
        
        // Create pipeline
        createPipeline();
        
        // Create vertex buffer
        createVertexBuffer();
        
        std::cout << "✓ Dawn WebGPU initialized successfully!\n";
        return true;
    }
    
    void render() {
        wgpu::TextureView textureView = swapChain.GetCurrentTextureView();
        if (!textureView) {
            std::cerr << "Failed to get texture view\n";
            return;
        }
        
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        
        wgpu::RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = textureView;
        colorAttachment.loadOp = wgpu::LoadOp::Clear;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        colorAttachment.clearValue = {0.2, 0.2, 0.3, 1.0};
        
        wgpu::RenderPassDescriptor renderPassDesc = {};
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);
        renderPass.SetPipeline(pipeline);
        renderPass.SetVertexBuffer(0, vertexBuffer);
        renderPass.Draw(3);
        renderPass.End();
        
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
        
        swapChain.Present();
    }
    
    void cleanup() {
        vertexBuffer = nullptr;
        pipeline = nullptr;
        shaderModule = nullptr;
        swapChain = nullptr;
        surface = nullptr;
        device = nullptr;
        instance.reset();
    }
    
private:
    void createPipeline() {
        // Vertex layout
        std::vector<wgpu::VertexAttribute> attributes(2);
        attributes[0].format = wgpu::VertexFormat::Float32x3;
        attributes[0].offset = 0;
        attributes[0].shaderLocation = 0;
        
        attributes[1].format = wgpu::VertexFormat::Float32x3;
        attributes[1].offset = 12;
        attributes[1].shaderLocation = 1;
        
        wgpu::VertexBufferLayout vertexLayout;
        vertexLayout.arrayStride = 24;
        vertexLayout.stepMode = wgpu::VertexStepMode::Vertex;
        vertexLayout.attributeCount = attributes.size();
        vertexLayout.attributes = attributes.data();
        
        // Pipeline layout
        wgpu::PipelineLayoutDescriptor layoutDesc = {};
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&layoutDesc);
        
        // Color target
        wgpu::ColorTargetState colorTarget = {};
        colorTarget.format = wgpu::TextureFormat::BGRA8Unorm;
        colorTarget.writeMask = wgpu::ColorWriteMask::All;
        
        wgpu::FragmentState fragmentState = {};
        fragmentState.module = shaderModule;
        fragmentState.entryPoint = "fs_main";
        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;
        
        // Create pipeline
        wgpu::RenderPipelineDescriptor pipelineDesc = {};
        pipelineDesc.layout = pipelineLayout;
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &vertexLayout;
        pipelineDesc.fragment = &fragmentState;
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
        pipelineDesc.primitive.cullMode = wgpu::CullMode::None;
        pipelineDesc.multisample.count = 1;
        
        pipeline = device.CreateRenderPipeline(&pipelineDesc);
    }
    
    void createVertexBuffer() {
        wgpu::BufferDescriptor bufferDesc = {};
        bufferDesc.size = sizeof(triangleVertices);
        bufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        
        vertexBuffer = device.CreateBuffer(&bufferDesc);
        device.GetQueue().WriteBuffer(vertexBuffer, 0, triangleVertices, sizeof(triangleVertices));
    }
    
    GLFWwindow* window = nullptr;
    std::unique_ptr<dawn::native::Instance> instance;
    wgpu::Device device;
    wgpu::Surface surface;
    wgpu::SwapChain swapChain;
    wgpu::ShaderModule shaderModule;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
};

int main() {
    std::cout << "Dawn WebGPU + GLFW Triangle Demo\n";
    std::cout << "==================================\n\n";
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Dawn WebGPU Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    
    DawnWebGPUApp app;
    if (!app.initialize(window)) {
        std::cerr << "Failed to initialize Dawn WebGPU\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    std::cout << "\nRendering colorful triangle with Dawn!\n";
    std::cout << "Press ESC to exit.\n";
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
        app.render();
    }
    
    app.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "\n✓ Dawn WebGPU demo complete!\n";
    return 0;
}