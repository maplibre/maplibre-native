// Simplified WebGPU Map Example for MapLibre Native
// This creates a window with WebGPU/Dawn and clears it to a color

#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <dawn/native/MetalBackend.h>
#include <webgpu/webgpu_cpp.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

#include <iostream>
#include <chrono>
#include <thread>

class WebGPUApp {
public:
    bool initialize(GLFWwindow* window) {
        this->window = window;

        // Initialize Dawn
        dawnProcSetProcs(&dawn::native::GetProcs());

        // Create instance
        instance = std::make_unique<dawn::native::Instance>();

        // Get Metal adapter
        wgpu::RequestAdapterOptions adapterOptions = {};
        adapterOptions.backendType = wgpu::BackendType::Metal;

        std::vector<dawn::native::Adapter> adapters = instance->EnumerateAdapters(&adapterOptions);
        if (adapters.empty()) {
            std::cerr << "No Metal adapter found\n";
            return false;
        }

        // Create device
        wgpu::DeviceDescriptor deviceDesc = {};
        device = wgpu::Device(adapters[0].CreateDevice(&deviceDesc));

        if (!device) {
            std::cerr << "Failed to create device\n";
            return false;
        }

        queue = device.GetQueue();

        // Create surface
        NSWindow* nsWindow = glfwGetCocoaWindow(window);
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        [nsWindow.contentView setLayer:metalLayer];
        [nsWindow.contentView setWantsLayer:YES];

        wgpu::SurfaceDescriptorFromMetalLayer metalDesc;
        metalDesc.layer = metalLayer;

        wgpu::SurfaceDescriptor surfaceDesc;
        surfaceDesc.nextInChain = &metalDesc;

        surface = wgpu::Surface(dawn::native::metal::CreateSurfaceFromMetalLayer(metalDesc.layer));

        // Configure surface
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        wgpu::SurfaceConfiguration config = {};
        config.device = device;
        config.usage = wgpu::TextureUsage::RenderAttachment;
        config.format = wgpu::TextureFormat::BGRA8Unorm;
        config.width = width;
        config.height = height;
        config.presentMode = wgpu::PresentMode::Fifo;
        config.alphaMode = wgpu::CompositeAlphaMode::Opaque;

        surface.Configure(&config);

        std::cout << "✓ WebGPU initialized with Metal backend (" << width << "x" << height << ")\n";
        return true;
    }

    void render() {
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);

        if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
            std::cerr << "Failed to get surface texture\n";
            return;
        }

        wgpu::TextureView view = surfaceTexture.texture.CreateView();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Clear to blue
        wgpu::RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = view;
        colorAttachment.loadOp = wgpu::LoadOp::Clear;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        colorAttachment.clearValue = {0.2f, 0.3f, 0.8f, 1.0f}; // Blue color

        wgpu::RenderPassDescriptor renderPassDesc = {};
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        surface.Present();
    }

    void cleanup() {
        device = nullptr;
        surface = nullptr;
    }

private:
    GLFWwindow* window = nullptr;
    std::unique_ptr<dawn::native::Instance> instance;
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::Surface surface;
};

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Configure for WebGPU
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "MapLibre WebGPU Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    // Initialize WebGPU
    WebGPUApp app;
    if (!app.initialize(window)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "WebGPU window created. Press ESC to exit.\n";
    std::cout << "The window should show a blue background.\n";

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        app.render();

        // Small delay to avoid burning CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Cleanup
    app.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Application closed.\n";
    return 0;
}
