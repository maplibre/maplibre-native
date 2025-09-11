#include "glfw_webgpu_backend.hpp"

#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/gfx/backend_scope.hpp>

#include <GLFW/glfw3.h>

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#include <dawn/native/MetalBackend.h>
#endif

#include <dawn/native/DawnNative.h>
#include <webgpu/webgpu_cpp.h>

#include <iostream>
#include <cassert>

namespace mbgl {
namespace gfx {
template <>
std::unique_ptr<GLFWBackend> Backend::Create<mbgl::gfx::Backend::Type::WebGPU>(GLFWwindow* window, bool capFrameRate) {
    return std::make_unique<GLFWWebGPUBackend>(window, capFrameRate);
}
} // namespace gfx
} // namespace mbgl

GLFWWebGPUBackend::GLFWWebGPUBackend(GLFWwindow* window_, bool capFrameRate)
    : GLFWBackend(),
      mbgl::webgpu::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::gfx::Renderable(mbgl::Size{0, 0}, nullptr),
      window(window_) {
    
    mbgl::Log::Info(mbgl::Event::General, "Initializing WebGPU backend");
    
    // Create Dawn instance
    instance = std::make_unique<dawn::native::Instance>();
    
    // Enumerate adapters
    std::vector<dawn::native::Adapter> adapters = instance->EnumerateAdapters();
    if (adapters.empty()) {
        mbgl::Log::Error(mbgl::Event::General, "No WebGPU adapters found");
        throw std::runtime_error("No WebGPU adapters found");
    }
    
    // Select first adapter (should be Metal on macOS)
    dawn::native::Adapter& selectedAdapter = adapters[0];
    mbgl::Log::Info(mbgl::Event::General, "Selected WebGPU adapter");
    
    // Create device
    device = reinterpret_cast<WGPUDeviceImpl*>(selectedAdapter.CreateDevice());
    if (!device) {
        mbgl::Log::Error(mbgl::Event::General, "Failed to create WebGPU device");
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
    metalLayer = (__bridge_retained void*)layer;
    
    // Create surface from metal layer
    wgpu::SurfaceDescriptorFromMetalLayer metalDesc;
    metalDesc.layer = metalLayer;
    
    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &metalDesc;
    
    // Create surface using the instance
    wgpu::Instance wgpuInstance(instance->Get());
    wgpu::Surface wgpuSurface = wgpuInstance.CreateSurface(&surfaceDesc);
    surface = reinterpret_cast<WGPUSurfaceImpl*>(wgpuSurface.Get());
    
    // Configure surface
    wgpu::SurfaceConfiguration config = {};
    config.device = wgpuDevice;
    config.format = wgpu::TextureFormat::BGRA8Unorm;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;
    config.width = width;
    config.height = height;
    config.presentMode = wgpu::PresentMode::Fifo;
    
    wgpuSurface.Configure(&config);
#endif
    
    // Store WebGPU device and queue in the base class
    setDevice(device);
    
    mbgl::Log::Info(mbgl::Event::General, "WebGPU backend initialized successfully");
}

GLFWWebGPUBackend::~GLFWWebGPUBackend() {
#ifdef __APPLE__
    if (metalLayer) {
        CFRelease(metalLayer);
        metalLayer = nullptr;
    }
#endif
}

void GLFWWebGPUBackend::swap() {
    // Present the current frame
    // TODO: Implement swap chain presentation
}

void GLFWWebGPUBackend::activate() {
    // WebGPU doesn't need explicit context activation like OpenGL
}

void GLFWWebGPUBackend::deactivate() {
    // WebGPU doesn't need explicit context deactivation like OpenGL
}

mbgl::Size GLFWWebGPUBackend::getSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

void GLFWWebGPUBackend::setSize(mbgl::Size newSize) {
    // Update swap chain size if needed
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    if (static_cast<uint32_t>(width) != newSize.width || 
        static_cast<uint32_t>(height) != newSize.height) {
#ifdef __APPLE__
        if (metalLayer) {
            CAMetalLayer* layer = (__bridge CAMetalLayer*)metalLayer;
            layer.drawableSize = CGSizeMake(newSize.width, newSize.height);
        }
#endif
        
        // Recreate swap chain with new size
        // TODO: Implement swap chain recreation
    }
}