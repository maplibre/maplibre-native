#include "glfw_webgpu_backend.hpp"

#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>

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


    // Create Dawn instance
    instance = std::make_unique<dawn::native::Instance>();

    // Enumerate adapters
    std::vector<dawn::native::Adapter> adapters = instance->EnumerateAdapters();
    if (adapters.empty()) {
        throw std::runtime_error("No WebGPU adapters found");
    }

    // Select first adapter (should be Metal on macOS)
    dawn::native::Adapter& selectedAdapter = adapters[0];

    // Create device with error callbacks
    WGPUDevice rawDevice = selectedAdapter.CreateDevice();
    if (!rawDevice) {
        throw std::runtime_error("Failed to create WebGPU device");
    }

    wgpuDevice = wgpu::Device::Acquire(rawDevice);

    // TODO: Add error callbacks once we figure out the correct Dawn API

#ifdef __APPLE__
    // Setup Metal surface on macOS
    NSWindow* nsWindow = glfwGetCocoaWindow(window);
    NSView* view = [nsWindow contentView];
    [view setWantsLayer:YES];

    // Create CAMetalLayer
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = dawn::native::metal::GetMTLDevice(wgpuDevice.Get());
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
    wgpuSurface = wgpuInstance.CreateSurface(&surfaceDesc);

    // Get the queue
    queue = wgpuDevice.GetQueue();

    // Configure surface
    wgpu::SurfaceConfiguration config = {};
    config.device = wgpuDevice;
    config.format = swapChainFormat;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;
    config.width = width;
    config.height = height;
    config.presentMode = wgpu::PresentMode::Fifo;

    wgpuSurface.Configure(&config);
#endif

    // Store WebGPU instance, device and queue in the base class
    setInstance(instance->Get());
    setDevice(reinterpret_cast<void*>(wgpuDevice.Get()));
    setQueue(reinterpret_cast<void*>(queue.Get()));

    // Make sure the window is visible and focused
    glfwShowWindow(window);
    glfwFocusWindow(window);

    // Check if window is actually visible
    if (glfwGetWindowAttrib(window, GLFW_VISIBLE) != GLFW_TRUE) {
    }

    // Make sure window shouldn't close
    if (glfwWindowShouldClose(window)) {
    }

}

GLFWWebGPUBackend::~GLFWWebGPUBackend() {
#ifdef __APPLE__
    if (metalLayer) {
        CFRelease(metalLayer);
        metalLayer = nullptr;
    }
#endif
}

mbgl::gfx::RendererBackend& GLFWWebGPUBackend::getRendererBackend() {
    // We inherit from mbgl::webgpu::RendererBackend which is a mbgl::gfx::RendererBackend
    return *this;
}

mbgl::gfx::Renderable& GLFWWebGPUBackend::getDefaultRenderable() {
    // We directly inherit from mbgl::gfx::Renderable
    return *this;
}



void GLFWWebGPUBackend::swap() {
    // Present the current frame
    if (wgpuSurface) {
        // The surface presentation happens automatically when the current
        // texture view goes out of scope and gets presented
        wgpuSurface.Present();

        // Clear the current texture view after presenting
        currentTextureView = nullptr;
    }

    // Poll for window events to keep the window responsive
    glfwPollEvents();

    // Note: We don't call glfwSwapBuffers for WebGPU as it doesn't use OpenGL swap chains
    // The presentation is handled by wgpuSurface.Present() above
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

void* GLFWWebGPUBackend::getCurrentTextureView() {

    wgpu::SurfaceTexture surfaceTexture;
    wgpuSurface.GetCurrentTexture(&surfaceTexture);

    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
        surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
        return nullptr;
    }

    if (!surfaceTexture.texture) {
        return nullptr;
    }

    // Store the texture view to keep it alive
    currentTextureView = surfaceTexture.texture.CreateView();
    return reinterpret_cast<void*>(currentTextureView.Get());
}

mbgl::Size GLFWWebGPUBackend::getFramebufferSize() const {
    return getSize();
}
