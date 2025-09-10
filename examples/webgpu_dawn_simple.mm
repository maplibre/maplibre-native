// Simple Dawn WebGPU Demo - Just creates device and clears screen
// This demonstrates Dawn is working

#include <iostream>
#include <memory>

// Dawn headers
#include <dawn/native/DawnNative.h>
#include <dawn/native/MetalBackend.h>
#include <webgpu/webgpu_cpp.h>

// GLFW headers
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// macOS headers
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

int main() {
    std::cout << "Dawn WebGPU Simple Demo\n";
    std::cout << "=======================\n\n";

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Dawn WebGPU - Clear Screen", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    std::cout << "✓ Created GLFW window\n";

    // Create Dawn instance
    auto instance = std::make_unique<dawn::native::Instance>();
    // Enumerate adapters is the new API

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

    // Use the first available adapter (should be Metal on macOS)
    dawn::native::Adapter& adapter = adapters[0];
    std::cout << "✓ Selected first adapter\n";

    // Create device
    WGPUDevice cDevice = adapter.CreateDevice();
    if (!cDevice) {
        std::cerr << "Failed to create device\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    wgpu::Device device = wgpu::Device::Acquire(cDevice);
    std::cout << "✓ Created WebGPU device\n";

    // Get queue
    wgpu::Queue queue = device.GetQueue();
    std::cout << "✓ Got device queue\n";

    // Set up Metal layer
    NSWindow* nsWindow = glfwGetCocoaWindow(window);
    NSView* view = [nsWindow contentView];
    [view setWantsLayer:YES];

    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.device = dawn::native::metal::GetMTLDevice(cDevice);
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.drawableSize = CGSizeMake(800, 600);
    [view setLayer:metalLayer];

    std::cout << "✓ Created Metal layer\n";

    // Create surface
    wgpu::SurfaceDescriptorFromMetalLayer metalDesc;
    metalDesc.layer = (__bridge void*)metalLayer;

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &metalDesc;

    wgpu::Surface surface = wgpu::Instance(instance->Get()).CreateSurface(&surfaceDesc);
    std::cout << "✓ Created surface\n";

    // Configure surface
    wgpu::SurfaceConfiguration config = {};
    config.device = device;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.format = wgpu::TextureFormat::BGRA8Unorm;
    config.width = 800;
    config.height = 600;
    config.presentMode = wgpu::PresentMode::Immediate;
    config.alphaMode = wgpu::CompositeAlphaMode::Opaque;

    surface.Configure(&config);
    std::cout << "✓ Configured surface\n\n";

    std::cout << "Dawn is working! Clearing screen with blue color.\n";
    std::cout << "Press ESC to exit.\n";

    // Main loop - just clear the screen
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Get current texture
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);

        if (surfaceTexture.texture) {
            wgpu::TextureView view = surfaceTexture.texture.CreateView();

            // Create command encoder
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            // Clear to blue
            wgpu::RenderPassColorAttachment colorAttachment = {};
            colorAttachment.view = view;
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = {0.2, 0.3, 0.8, 1.0}; // Blue color

            wgpu::RenderPassDescriptor renderPassDesc = {};
            renderPassDesc.colorAttachmentCount = 1;
            renderPassDesc.colorAttachments = &colorAttachment;

            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
            pass.End();

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            surface.Present();
        }
    }

    std::cout << "\n✓ Dawn WebGPU working successfully!\n";
    std::cout << "The WebGPU backend is ready for MapLibre Native!\n";

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
