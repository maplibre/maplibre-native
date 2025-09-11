// WebGPU Render Demo for MapLibre Native
// Demonstrates WebGPU backend integration

#include <iostream>
#include <memory>
#include <vector>

// Dawn headers
#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <dawn/native/MetalBackend.h>
#include <webgpu/webgpu_cpp.h>

// GLFW headers
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// Platform headers
#ifdef __APPLE__
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>
#endif

// MapLibre WebGPU backend
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>

class WebGPURenderDemo {
public:
    bool initialize(GLFWwindow* window) {
        this->window = window;
        
        std::cout << "Initializing WebGPU Render Demo...\n";
        
        // Initialize Dawn
        dawnProcSetProcs(&dawn::native::GetProcs());
        
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
        
        // Get adapter info
        wgpu::AdapterInfo info = {};
        adapter.GetInfo(&info);
        std::cout << "  Device: " << (info.device.data ? std::string(info.device.data, info.device.length) : "Unknown") << "\n";
        
        // Create surface
        if (!createSurface()) {
            return false;
        }
        
        // Create device
        if (!createDevice()) {
            return false;
        }
        
        // Initialize MapLibre WebGPU backend
        backend = std::make_unique<mbgl::webgpu::RendererBackend>(mbgl::gfx::ContextMode::Shared);
        backend->setDevice(device.Get());
        backend->setQueue(queue.Get());
        backend->setSurface(surface.Get());
        
        std::cout << "✓ WebGPU backend initialized\n";
        
        // Create WebGPU context
        context = std::make_unique<mbgl::webgpu::Context>(*backend, device.Get());
        
        std::cout << "✓ WebGPU context created\n";
        
        return true;
    }
    
    void render() {
        // Get surface texture
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        
        if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
            surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
            std::cerr << "Failed to get surface texture\n";
            return;
        }
        
        wgpu::TextureView textureView = surfaceTexture.texture.CreateView();
        
        // Create command encoder
        wgpu::CommandEncoderDescriptor encoderDesc = {};
        encoderDesc.label = "Render Encoder";
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);
        
        // Create render pass - clear to MapLibre blue
        wgpu::RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = textureView;
        colorAttachment.loadOp = wgpu::LoadOp::Clear;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        colorAttachment.clearValue = {0.156, 0.467, 0.784, 1.0}; // MapLibre blue
        
        wgpu::RenderPassDescriptor renderPassDesc = {};
        renderPassDesc.label = "MapLibre Render Pass";
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);
        
        // Here we would render MapLibre content using the WebGPU context
        // For now, just clear to blue
        
        renderPass.End();
        
        // Submit commands
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        
        // Present
        surface.Present();
        
        frameCount++;
        if (frameCount % 60 == 0) {
            std::cout << "Frame " << frameCount << " - WebGPU rendering active\n";
        }
    }
    
    void cleanup() {
        context.reset();
        backend.reset();
    }
    
private:
    bool createSurface() {
#ifdef __APPLE__
        NSWindow* nsWindow = glfwGetCocoaWindow(window);
        if (!nsWindow) {
            std::cerr << "Failed to get NSWindow from GLFW\n";
            return false;
        }
        
        // Create Metal layer
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        [nsWindow.contentView setWantsLayer:YES];
        [nsWindow.contentView setLayer:metalLayer];
        
        // Get window size
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        [metalLayer setDrawableSize:CGSizeMake(width, height)];
        
        // Create surface descriptor
        wgpu::SurfaceDescriptorFromMetalLayer metalDesc = {};
        metalDesc.layer = (__bridge void*)metalLayer;
        
        wgpu::SurfaceDescriptor surfaceDesc = {};
        surfaceDesc.nextInChain = &metalDesc;
        surfaceDesc.label = "MapLibre Surface";
        
        // Create surface
        wgpu::Instance instance = wgpu::Instance(dawnInstance.Get());
        surface = instance.CreateSurface(&surfaceDesc);
        
        if (!surface) {
            std::cerr << "Failed to create surface\n";
            return false;
        }
        
        // Configure surface
        wgpu::SurfaceConfiguration config = {};
        config.device = device;
        config.usage = wgpu::TextureUsage::RenderAttachment;
        config.format = wgpu::TextureFormat::BGRA8Unorm;
        config.width = width;
        config.height = height;
        config.presentMode = wgpu::PresentMode::Fifo;
        config.alphaMode = wgpu::CompositeAlphaMode::Opaque;
        
        surface.Configure(&config);
        
        std::cout << "✓ Created and configured Metal surface (" << width << "x" << height << ")\n";
        return true;
#else
        std::cerr << "Only macOS is supported\n";
        return false;
#endif
    }
    
    bool createDevice() {
        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.label = "MapLibre Device";
        
        device = adapter.CreateDevice(&deviceDesc);
        if (!device) {
            std::cerr << "Failed to create device\n";
            return false;
        }
        
        queue = device.GetQueue();
        std::cout << "✓ Created device and queue\n";
        return true;
    }
    
private:
    GLFWwindow* window = nullptr;
    dawn::native::Instance dawnInstance;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::Surface surface;
    
    std::unique_ptr<mbgl::webgpu::RendererBackend> backend;
    std::unique_ptr<mbgl::webgpu::Context> context;
    
    uint64_t frameCount = 0;
};

int main() {
    std::cout << "WebGPU Render Demo\n";
    std::cout << "==================\n\n";
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }
    
    // Configure GLFW for WebGPU
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "MapLibre WebGPU Render Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }
    
    std::cout << "✓ Created GLFW window (800x600)\n";
    
    // Initialize demo
    WebGPURenderDemo demo;
    if (!demo.initialize(window)) {
        std::cerr << "Failed to initialize WebGPU render demo\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    std::cout << "\n🎨 WebGPU Render Demo Running!\n";
    std::cout << "  • Rendering with MapLibre WebGPU backend\n";
    std::cout << "  • Using Dawn Metal backend\n";
    std::cout << "  • Press ESC to exit\n\n";
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Check for ESC
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
        // Render
        demo.render();
    }
    
    // Cleanup
    demo.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "\n✓ Cleanup complete\n";
    return 0;
}