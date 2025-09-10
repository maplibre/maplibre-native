#include "glfw_webgpu_backend.hpp"

#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>

#include <GLFW/glfw3.h>

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#include <dawn/native/MetalBackend.h>
#endif

#include <dawn/native/DawnNative.h>
#include <dawn/webgpu_cpp.h>

#include <iostream>
#include <cassert>

class GLFWWebGPUBackend::Impl {
public:
    std::unique_ptr<dawn::native::Instance> instance;
    dawn::native::Adapter adapter;
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::Surface surface;
    wgpu::SwapChain swapChain;
    
    mbgl::Size framebufferSize;
};

GLFWWebGPUBackend::GLFWWebGPUBackend(GLFWwindow* window_, bool capFrameRate)
    : mbgl::webgpu::RendererBackend(mbgl::gfx::ContextMode::Unique),
      window(window_) {
    
    mbgl::Log::Info(mbgl::Event::WebGPU, "Initializing WebGPU backend");
    
    // Create Dawn instance
    instance = std::make_unique<dawn::native::Instance>();
    
    // Enumerate adapters
    std::vector<dawn::native::Adapter> adapters = instance->EnumerateAdapters();
    if (adapters.empty()) {
        mbgl::Log::Error(mbgl::Event::WebGPU, "No WebGPU adapters found");
        throw std::runtime_error("No WebGPU adapters found");
    }
    
    // Select first adapter (should be Metal on macOS)
    dawn::native::Adapter& selectedAdapter = adapters[0];
    mbgl::Log::Info(mbgl::Event::WebGPU, "Selected WebGPU adapter");
    
    // Create device
    device = reinterpret_cast<WGPUDeviceImpl*>(selectedAdapter.CreateDevice());
    if (!device) {
        mbgl::Log::Error(mbgl::Event::WebGPU, "Failed to create WebGPU device");
        throw std::runtime_error("Failed to create WebGPU device");
    }
    
    wgpu::Device wgpuDevice = wgpu::Device::Acquire(reinterpret_cast<WGPUDevice>(device));
    
#ifdef __APPLE__
    // Setup Metal surface on macOS
    NSWindow* nsWindow = glfwGetCocoaWindow(window);
    NSView* view = [nsWindow contentView];
    [view setWantsLayer:YES];
    
    // Create CAMetalLayer
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = dawn::native::metal::GetMTLDevice(reinterpret_cast<WGPUDevice>(device));
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    // Get window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    layer.drawableSize = CGSizeMake(width, height);
    
    [view setLayer:layer];
    metalLayer = layer;
    
    // Create surface from metal layer
    wgpu::SurfaceDescriptorFromMetalLayer metalDesc;
    metalDesc.layer = metalLayer;
    
    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &metalDesc;
    
    surface = reinterpret_cast<WGPUSurfaceImpl*>(
        instance->CreateSurface(&surfaceDesc).Release());
#endif
    
    // Configure swap chain
    wgpu::SwapChainDescriptor swapChainDesc = {};
    swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
    swapChainDesc.format = wgpu::TextureFormat::BGRA8Unorm;
    swapChainDesc.width = width;
    swapChainDesc.height = height;
    swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
    
    wgpu::Surface wgpuSurface(reinterpret_cast<WGPUSurface>(surface));
    auto swapChain = wgpuDevice.CreateSwapChain(wgpuSurface, &swapChainDesc);
    
    mbgl::Log::Info(mbgl::Event::WebGPU, 
                    mbgl::util::toString(width) + "x" + mbgl::util::toString(height) + 
                    " WebGPU context created successfully");
}

GLFWWebGPUBackend::~GLFWWebGPUBackend() {
    deactivate();
    
#ifdef __APPLE__
    if (metalLayer) {
        CAMetalLayer* layer = static_cast<CAMetalLayer*>(metalLayer);
        [layer removeFromSuperlayer];
    }
#endif
}

void GLFWWebGPUBackend::activate() {
    // WebGPU doesn't need explicit context activation like OpenGL
}

void GLFWWebGPUBackend::deactivate() {
    // WebGPU doesn't need explicit context deactivation like OpenGL
}

void GLFWWebGPUBackend::updateAssumedState() {
    // Update any assumed WebGPU state if needed
}

mbgl::Size GLFWWebGPUBackend::getSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

void GLFWWebGPUBackend::setSize(mbgl::Size size) {
    // Update swap chain size if needed
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    if (static_cast<uint32_t>(width) != size.width || 
        static_cast<uint32_t>(height) != size.height) {
        
#ifdef __APPLE__
        if (metalLayer) {
            CAMetalLayer* layer = static_cast<CAMetalLayer*>(metalLayer);
            layer.drawableSize = CGSizeMake(size.width, size.height);
        }
#endif
        
        // Recreate swap chain with new size
        wgpu::Device wgpuDevice = wgpu::Device::Acquire(reinterpret_cast<WGPUDevice>(device));
        wgpu::Surface wgpuSurface(reinterpret_cast<WGPUSurface>(surface));
        
        wgpu::SwapChainDescriptor swapChainDesc = {};
        swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
        swapChainDesc.format = wgpu::TextureFormat::BGRA8Unorm;
        swapChainDesc.width = size.width;
        swapChainDesc.height = size.height;
        swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
        
        wgpuDevice.CreateSwapChain(wgpuSurface, &swapChainDesc);
    }
}

void GLFWWebGPUBackend::swap() {
    // Present the swap chain
    wgpu::Device wgpuDevice = wgpu::Device::Acquire(reinterpret_cast<WGPUDevice>(device));
    wgpu::Surface wgpuSurface(reinterpret_cast<WGPUSurface>(surface));
    
    // In a real implementation, we'd present the rendered frame here
    // For now, just poll events
    glfwPollEvents();
}