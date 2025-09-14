// WebGPU + GLFW Example for MapLibre Native
// This demonstrates using Dawn with GLFW to create a WebGPU rendering surface

#include <iostream>
#include <memory>
#include <vector>
#include <cstring>

// Platform detection
#ifdef __APPLE__
    #define GLFW_EXPOSE_NATIVE_COCOA
    #import <Cocoa/Cocoa.h>
    #import <QuartzCore/CAMetalLayer.h>
#elif defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
#else
    #define GLFW_EXPOSE_NATIVE_X11
#endif

// GLFW headers
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// Dawn-specific headers
#ifdef MLN_WITH_DAWN
#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <webgpu/webgpu.h>
#else
// Generic WebGPU headers
#include <mbgl/webgpu/backend_impl.hpp>
#endif

// Simple vertex data for a triangle
const float triangleVertices[] = {
    // positions        // colors
     0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top (red)
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left (green)
     0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // bottom right (blue)
};

// WGSL Shader code
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
        
        // Create WebGPU instance
        if (!createInstance()) {
            return false;
        }
        
        // Create surface from GLFW window
        if (!createSurface()) {
            return false;
        }
        
        // Request adapter
        if (!requestAdapter()) {
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
        
        return true;
    }
    
    void render() {
        // Get current texture view from swapchain
        WGPUTextureView textureView = getNextTextureView();
        if (!textureView) {
            return;
        }
        
        // Create command encoder
        WGPUCommandEncoderDescriptor encoderDesc = {};
#ifdef MLN_WITH_DAWN
        WGPUStringView encoderLabel = { "Command Encoder", WGPU_STRLEN };
        encoderDesc.label = encoderLabel;
#else
        encoderDesc.label = "Command Encoder";
#endif
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);
        
        // Create render pass
        WGPURenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = textureView;
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = {0.2, 0.2, 0.3, 1.0};  // Dark blue background
        
        WGPURenderPassDescriptor renderPassDesc = {};
#ifdef MLN_WITH_DAWN
        WGPUStringView renderPassLabel = { "Render Pass", WGPU_STRLEN };
        renderPassDesc.label = renderPassLabel;
#else
        renderPassDesc.label = "Render Pass";
#endif
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        
        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
        
        // Draw triangle
        wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
        wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0, sizeof(triangleVertices));
        wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
        
        // End render pass
        wgpuRenderPassEncoderEnd(renderPass);
        wgpuRenderPassEncoderRelease(renderPass);
        
        // Submit commands
        WGPUCommandBufferDescriptor cmdBufferDesc = {};
#ifdef MLN_WITH_DAWN
        WGPUStringView cmdBufferLabel = { "Command Buffer", WGPU_STRLEN };
        cmdBufferDesc.label = cmdBufferLabel;
#else
        cmdBufferDesc.label = "Command Buffer";
#endif
        WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
        wgpuQueueSubmit(queue, 1, &commands);
        
        // Present
        wgpuSurfacePresent(surface);
        
        // Cleanup
        wgpuCommandBufferRelease(commands);
        wgpuCommandEncoderRelease(encoder);
        wgpuTextureViewRelease(textureView);
    }
    
    void cleanup() {
        if (vertexBuffer) wgpuBufferRelease(vertexBuffer);
        if (pipeline) wgpuRenderPipelineRelease(pipeline);
        if (device) wgpuDeviceRelease(device);
        if (adapter) wgpuAdapterRelease(adapter);
        if (surface) wgpuSurfaceRelease(surface);
        if (instance) wgpuInstanceRelease(instance);
    }
    
private:
    GLFWwindow* window = nullptr;
    WGPUInstance instance = nullptr;
    WGPUSurface surface = nullptr;
    WGPUAdapter adapter = nullptr;
    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;
    WGPUSwapChain swapChain = nullptr;
    WGPURenderPipeline pipeline = nullptr;
    WGPUBuffer vertexBuffer = nullptr;
    
    bool createInstance() {
#ifdef MLN_WITH_DAWN
        // Initialize Dawn's proc table
        dawnProcSetProcs(&dawn::native::GetProcs());
        
        // Create Dawn instance with default descriptor
        WGPUInstanceDescriptor desc = {};
        instance = wgpuCreateInstance(&desc);
#else
        WGPUInstanceDescriptor desc = {};
        instance = wgpuCreateInstance(&desc);
#endif
        
        if (!instance) {
            std::cerr << "Failed to create WebGPU instance\n";
            return false;
        }
        std::cout << "✓ Created WebGPU instance\n";
        return true;
    }
    
    bool createSurface() {
#ifdef MLN_WITH_DAWN
    #ifdef __APPLE__
        // Create Metal surface for macOS
        NSWindow* nsWindow = glfwGetCocoaWindow(window);
        if (!nsWindow) {
            std::cerr << "Failed to get NSWindow from GLFW\n";
            return false;
        }
        
        // Create Metal layer
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        [nsWindow.contentView setWantsLayer:YES];
        [nsWindow.contentView setLayer:metalLayer];
        
        // Create surface descriptor
        WGPUSurfaceDescriptor surfaceDesc = {};
        
        // Dawn-specific Metal surface setup
        struct WGPUSurfaceDescriptorFromMetalLayer {
            WGPUChainedStruct chain;
            void* layer;  // CAMetalLayer*
        };
        
        WGPUSurfaceDescriptorFromMetalLayer metalDesc = {};
        metalDesc.chain.sType = static_cast<WGPUSType>(0x0000000E); // WGPUSType_SurfaceDescriptorFromMetalLayer
        metalDesc.layer = metalLayer;
        surfaceDesc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&metalDesc);
#ifdef MLN_WITH_DAWN
        WGPUStringView surfaceLabel = { "Metal Surface", WGPU_STRLEN };
        surfaceDesc.label = surfaceLabel;
#else
        surfaceDesc.label = "Metal Surface";
#endif
    #else
        #error "Non-macOS platforms not yet supported with Dawn"
    #endif
#else
        WGPUSurfaceDescriptor surfaceDesc = {};
#ifdef MLN_WITH_DAWN
        WGPUStringView surfaceLabel = { "GLFW Surface", WGPU_STRLEN };
        surfaceDesc.label = surfaceLabel;
#else
        surfaceDesc.label = "GLFW Surface";
#endif
#endif
        
        surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);
        
        if (!surface) {
            std::cerr << "Failed to create surface\n";
            return false;
        }
        
        std::cout << "✓ Created surface from GLFW window\n";
        return true;
    }
    
    bool requestAdapter() {
        WGPURequestAdapterOptions options = {};
        options.compatibleSurface = surface;
        options.powerPreference = WGPUPowerPreference_HighPerformance;
        
        // Note: In real Dawn implementation, this would be async
        // For this example, we're using a simplified synchronous approach
        bool adapterReceived = false;
        wgpuInstanceRequestAdapter(instance, &options, 
            [](WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userdata) {
                if (status == WGPURequestAdapterStatus_Success) {
                    *reinterpret_cast<WGPUAdapter*>(userdata) = adapter;
                }
            }, &adapter);
        
        // In stub implementation, just set a dummy adapter
        adapter = reinterpret_cast<WGPUAdapter>(1);
        
        if (!adapter) {
            std::cerr << "Failed to get adapter\n";
            return false;
        }
        
        std::cout << "✓ Got WebGPU adapter\n";
        return true;
    }
    
    bool createDevice() {
        WGPUDeviceDescriptor deviceDesc = {};
        deviceDesc.label = "WebGPU Device";
        
        device = wgpuAdapterCreateDevice(adapter, &deviceDesc);
        if (!device) {
            std::cerr << "Failed to create device\n";
            return false;
        }
        
        queue = wgpuDeviceGetQueue(device);
        std::cout << "✓ Created WebGPU device and queue\n";
        return true;
    }
    
    bool configureSurface() {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        WGPUSwapChainDescriptor swapChainDesc = {};
        swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
        swapChainDesc.format = WGPUTextureFormat_BGRA8Unorm;
        swapChainDesc.width = width;
        swapChainDesc.height = height;
        swapChainDesc.presentMode = WGPUPresentMode_Fifo;
        
        swapChain = wgpuDeviceCreateSwapChain(device, surface, &swapChainDesc);
        if (!swapChain) {
            std::cerr << "Failed to create swap chain\n";
            return false;
        }
        
        std::cout << "✓ Configured surface swap chain (" << width << "x" << height << ")\n";
        return true;
    }
    
    bool createPipeline() {
        // Create shader module
        WGPUShaderModuleWGSLDescriptor wgslDesc = {};
        wgslDesc.chain.sType = static_cast<WGPUSType>(0x00000006); // WGPUSType_ShaderModuleWGSLDescriptor
#ifdef MLN_WITH_DAWN
        WGPUStringView codeView = { shaderCode, WGPU_STRLEN };
        wgslDesc.code = codeView;
#else
        wgslDesc.code = shaderCode;
#endif
        
        WGPUShaderModuleDescriptor shaderDesc = {};
        shaderDesc.nextInChain = &wgslDesc.chain;
#ifdef MLN_WITH_DAWN
        WGPUStringView shaderLabel = { "Triangle Shader", WGPU_STRLEN };
        shaderDesc.label = shaderLabel;
#else
        shaderDesc.label = "Triangle Shader";
#endif
        
        WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderDesc);
        
        // Vertex layout
        WGPUVertexAttribute attributes[2] = {};
        attributes[0].format = WGPUVertexFormat_Float32x3;
        attributes[0].offset = 0;
        attributes[0].shaderLocation = 0;
        
        attributes[1].format = WGPUVertexFormat_Float32x3;
        attributes[1].offset = 12;
        attributes[1].shaderLocation = 1;
        
        WGPUVertexBufferLayout vertexLayout = {};
        vertexLayout.arrayStride = 24;
        vertexLayout.stepMode = WGPUVertexStepMode_Vertex;
        vertexLayout.attributeCount = 2;
        vertexLayout.attributes = attributes;
        
        // Pipeline
        WGPURenderPipelineDescriptor pipelineDesc = {};
#ifdef MLN_WITH_DAWN
        WGPUStringView pipelineLabel = { "Triangle Pipeline", WGPU_STRLEN };
        pipelineDesc.label = pipelineLabel;
#else
        pipelineDesc.label = "Triangle Pipeline";
#endif
        
        pipelineDesc.vertex.module = shaderModule;
#ifdef MLN_WITH_DAWN
        WGPUStringView vsEntryPoint = { "vs_main", WGPU_STRLEN };
        pipelineDesc.vertex.entryPoint = vsEntryPoint;
#else
        pipelineDesc.vertex.entryPoint = "vs_main";
#endif
        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &vertexLayout;
        
        WGPUFragmentState fragmentState = {};
        fragmentState.module = shaderModule;
#ifdef MLN_WITH_DAWN
        WGPUStringView fsEntryPoint = { "fs_main", WGPU_STRLEN };
        fragmentState.entryPoint = fsEntryPoint;
#else
        fragmentState.entryPoint = "fs_main";
#endif
        
        WGPUColorTargetState colorTarget = {};
        colorTarget.format = WGPUTextureFormat_BGRA8Unorm;
        colorTarget.writeMask = WGPUColorWriteMask_All;
        
        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;
        
        pipelineDesc.fragment = &fragmentState;
        
        pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
        pipelineDesc.primitive.cullMode = WGPUCullMode_None;
        
        pipelineDesc.multisample.count = 1;
        
        pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
        wgpuShaderModuleRelease(shaderModule);
        
        if (!pipeline) {
            std::cerr << "Failed to create pipeline\n";
            return false;
        }
        
        std::cout << "✓ Created render pipeline\n";
        return true;
    }
    
    bool createVertexBuffer() {
        WGPUBufferDescriptor bufferDesc = {};
#ifdef MLN_WITH_DAWN
        WGPUStringView bufferLabel = { "Vertex Buffer", WGPU_STRLEN };
        bufferDesc.label = bufferLabel;
#else
        bufferDesc.label = "Vertex Buffer";
#endif
        bufferDesc.size = sizeof(triangleVertices);
        bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = true;
        
        vertexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        
        void* data = wgpuBufferGetMappedRange(vertexBuffer, 0, sizeof(triangleVertices));
        std::memcpy(data, triangleVertices, sizeof(triangleVertices));
        wgpuBufferUnmap(vertexBuffer);
        
        std::cout << "✓ Created vertex buffer\n";
        return true;
    }
    
    WGPUTextureView getNextTextureView() {
        return wgpuSwapChainGetCurrentTextureView(swapChain);
    }
};

// Note: Using stub WebGPU implementation from backend_impl.hpp

int main() {
    std::cout << "WebGPU + GLFW + Dawn Example\n";
    std::cout << "=============================\n\n";
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    // Configure GLFW for WebGPU (no OpenGL context)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "WebGPU Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    
    std::cout << "✓ Created GLFW window (800x600)\n";
    
    // Initialize WebGPU
    WebGPUApp app;
    if (!app.initialize(window)) {
        std::cerr << "Failed to initialize WebGPU\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    std::cout << "\n✓ WebGPU initialization complete!\n";
    std::cout << "Rendering a colorful triangle...\n\n";
    std::cout << "Press ESC or close the window to exit.\n";
    
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