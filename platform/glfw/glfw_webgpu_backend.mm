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
#include <thread>
#include <chrono>

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
    

    // Add small delay to let previous resources clean up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));


    // Create Dawn instance
    instance = std::make_unique<dawn::native::Instance>();

    // Enumerate adapters with retry logic
    std::vector<dawn::native::Adapter> adapters;
    for (int attempt = 0; attempt < 3; ++attempt) {
        adapters = instance->EnumerateAdapters();
        if (!adapters.empty()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (adapters.empty()) {
        throw std::runtime_error("No WebGPU adapters found after retries");
    }

    // Select first adapter (should be Metal on macOS)
    dawn::native::Adapter& selectedAdapter = adapters[0];

    // Set up device descriptor with error callback
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = "MapLibre WebGPU Device";

    // Request features we need
    std::vector<wgpu::FeatureName> requiredFeatures;
    deviceDesc.requiredFeatures = requiredFeatures.data();
    deviceDesc.requiredFeatureCount = requiredFeatures.size();

    // Create device with descriptor
    WGPUDevice rawDevice = selectedAdapter.CreateDevice(&deviceDesc);
    if (!rawDevice) {
        // Retry once after delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        rawDevice = selectedAdapter.CreateDevice(&deviceDesc);
        if (!rawDevice) {
            throw std::runtime_error("Failed to create WebGPU device after retry");
        }
    }

    wgpuDevice = wgpu::Device::Acquire(rawDevice);

    // Process device events to ensure it's ready
    wgpuDeviceTick(rawDevice);

    // Note: Dawn's error callback API is different from wgpu-native
    // We can set callbacks on the raw device if needed
    // For now, errors will be logged by Dawn's internal validation

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
    surfaceConfigured = true;
    lastConfiguredSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
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

    // Final delay to ensure everything is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

GLFWWebGPUBackend::~GLFWWebGPUBackend() {
    // Force frame completion to unblock any waiting threads
    frameInProgress = false;
    signalFrameComplete();
    
    // Give time for any pending operations to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Release current texture view and texture if any
    {
        std::lock_guard<std::mutex> lock(textureViewMutex);
        currentTextureView = nullptr;
        currentTexture = nullptr;
        
        // Clear deferred cleanup queue
        while (!deferredCleanup.empty()) {
            deferredCleanup.pop();
        }
    }
    
    // Process final device work
    if (wgpuDevice) {
        wgpuDeviceTick(wgpuDevice.Get());
    }

    // Unconfigure surface before releasing
    if (wgpuSurface && surfaceConfigured) {
        wgpuSurface.Unconfigure();
        surfaceConfigured = false;
    }

    // Release surface
    wgpuSurface = nullptr;

    // Release queue
    queue = nullptr;

    // Release device
    wgpuDevice = nullptr;

#ifdef __APPLE__
    if (metalLayer) {
        CFRelease(metalLayer);
        metalLayer = nullptr;
    }
#endif

    // Release instance last
    instance.reset();
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
    // Process any pending device work
    if (wgpuDevice) {
        wgpuDeviceTick(wgpuDevice.Get());
    }
    
    // Wait for any previous frame to complete with longer timeout
    if (!waitForFrame(std::chrono::milliseconds(500))) {
        // Timeout - force frame completion
        signalFrameComplete();
    }
    
    // Present the current frame
    if (wgpuSurface && surfaceConfigured) {
        // Check if surface needs reconfiguration
        if (surfaceNeedsReconfigure) {
            reconfigureSurface();
        }
        
        // Lock for texture view manipulation
        {
            std::lock_guard<std::mutex> lock(textureViewMutex);
            
            // Only present if we have a valid texture view and haven't presented yet
            if (currentTextureView && !framePresented) {
                // Validate the texture view before presenting
                WGPUTextureView raw = currentTextureView.Get();
                if (raw != nullptr) {
                    uintptr_t ptr = reinterpret_cast<uintptr_t>(raw);
                    if (ptr > 0x1000 && ptr < 0x0001000000000000ULL) {
                        // Valid pointer, safe to present
                        wgpuSurface.Present();
                        framePresented = true;
                    }
                }
                
                // Move current resources to deferred cleanup
                // They will be released after a few frames when GPU is done with them
                if (currentTextureView && currentTexture) {
                    deferredCleanup.push({std::move(currentTextureView), std::move(currentTexture)});
                    
                    // Clean up old frames that are guaranteed to be done
                    while (deferredCleanup.size() > maxDeferredFrames) {
                        deferredCleanup.pop();
                    }
                }
                
                // Clear current references (resources are now in deferred cleanup)
                currentTextureView = nullptr;
                currentTexture = nullptr;
            }
        }
        
        // Reset error counter on successful present
        consecutiveErrors = 0;
    }
    
    // Signal frame completion
    signalFrameComplete();

    // Poll for window events to keep the window responsive
    glfwPollEvents();
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

        // Mark surface for reconfiguration
        surfaceNeedsReconfigure = true;
    }
}

void* GLFWWebGPUBackend::getCurrentTextureView() {
    if (!wgpuSurface || !surfaceConfigured) {
        return nullptr;
    }
    
    // Process any pending device work before acquiring texture
    if (wgpuDevice) {
        wgpuDeviceTick(wgpuDevice.Get());
    }
    
    // Check error threshold
    if (consecutiveErrors >= maxConsecutiveErrors) {
        // Too many errors, need to reconfigure
        surfaceNeedsReconfigure = true;
        reconfigureSurface();
        consecutiveErrors = 0;
    }

    // Lock to protect texture view access
    std::lock_guard<std::mutex> lock(textureViewMutex);
    
    // If we already have a current texture view and haven't presented, return it
    if (currentTextureView && !framePresented) {
        // Ensure it's still valid
        WGPUTextureView raw = currentTextureView.Get();
        if (raw != nullptr) {
            // Validate pointer range - ARM64 uses 48-bit virtual addresses
            uintptr_t ptr = reinterpret_cast<uintptr_t>(raw);
            if (ptr > 0x1000 && ptr < 0x0001000000000000ULL) {
                return reinterpret_cast<void*>(raw);
            }
        }
        // Invalid, clear both texture and view
        currentTextureView = nullptr;
        currentTexture = nullptr;
    }
    
    // Mark frame as in progress
    bool expected = false;
    if (!frameInProgress.compare_exchange_strong(expected, true)) {
        // Another frame is already in progress, skip this request
        return nullptr;
    }
    
    // Mark that we haven't presented this frame yet
    framePresented = false;
    
    // Acquire new surface texture with additional safety
    wgpu::SurfaceTexture surfaceTexture;
    try {
        wgpuSurface.GetCurrentTexture(&surfaceTexture);
    } catch (...) {
        // Handle any exceptions during texture acquisition
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }

    // Handle various error states
    if (surfaceTexture.status == wgpu::SurfaceGetCurrentTextureStatus::Timeout ||
        surfaceTexture.status == wgpu::SurfaceGetCurrentTextureStatus::Outdated ||
        surfaceTexture.status == wgpu::SurfaceGetCurrentTextureStatus::Lost) {
        // Surface needs reconfiguration
        surfaceNeedsReconfigure = true;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }
    
    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
        surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
        // Some other error
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }

    if (!surfaceTexture.texture) {
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }

    // Validate the texture pointer before creating a view
    WGPUTexture rawTexture = surfaceTexture.texture.Get();
    if (rawTexture == nullptr) {
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }
    
    // Validate pointer is in valid ARM64 address range
    uintptr_t texPtr = reinterpret_cast<uintptr_t>(rawTexture);
    if (texPtr <= 0x1000 || texPtr >= 0x0001000000000000ULL) {
        // Invalid pointer range
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }

    // Create texture view with explicit descriptor
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.format = swapChainFormat;
    viewDesc.dimension = wgpu::TextureViewDimension::e2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = wgpu::TextureAspect::All;
    viewDesc.label = "SwapChain TextureView";
    
    // Store the texture to keep it alive
    currentTexture = surfaceTexture.texture;
    
    try {
        currentTextureView = currentTexture.CreateView(&viewDesc);
    } catch (...) {
        // Handle any exceptions from Dawn
        currentTexture = nullptr;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }
    
    if (!currentTextureView) {
        currentTexture = nullptr;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }
    
    // Final validation of the created texture view
    WGPUTextureView raw = currentTextureView.Get();
    if (raw == nullptr) {
        currentTextureView = nullptr;
        currentTexture = nullptr;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }
    
    // Validate the pointer is in valid ARM64 48-bit address space
    uintptr_t ptr = reinterpret_cast<uintptr_t>(raw);
    if (ptr <= 0x1000 || ptr >= 0x0001000000000000ULL) {
        // Pointer outside valid range - likely corrupted
        currentTextureView = nullptr;
        currentTexture = nullptr;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }
    
    // Success - reset error counter
    consecutiveErrors = 0;
    
    // Memory barrier to ensure all writes are visible
    std::atomic_thread_fence(std::memory_order_release);
    
    return reinterpret_cast<void*>(raw);
}

mbgl::Size GLFWWebGPUBackend::getFramebufferSize() const {
    return getSize();
}

void GLFWWebGPUBackend::reconfigureSurface() {
    if (!wgpuSurface || !wgpuDevice) {
        return;
    }
    
    // Wait for any in-progress frame
    waitForFrame();
    
    // Lock to ensure exclusive access
    std::lock_guard<std::mutex> lock(textureViewMutex);
    
    // Clear current state
    currentTextureView = nullptr;
    currentTexture = nullptr;
    
    // Clear deferred cleanup
    while (!deferredCleanup.empty()) {
        deferredCleanup.pop();
    }
    
    // Get current size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    // Skip if size hasn't changed and we're already configured
    if (surfaceConfigured && 
        lastConfiguredSize.width == static_cast<uint32_t>(width) &&
        lastConfiguredSize.height == static_cast<uint32_t>(height)) {
        surfaceNeedsReconfigure = false;
        return;
    }
    
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
    
    // Update state
    surfaceConfigured = true;
    surfaceNeedsReconfigure = false;
    lastConfiguredSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    consecutiveErrors = 0;
}

bool GLFWWebGPUBackend::waitForFrame(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(frameMutex);
    if (!frameInProgress) {
        return true;  // No frame in progress
    }
    
    // Wait for frame to complete with timeout
    return frameCV.wait_for(lock, timeout, [this] { return !frameInProgress.load(); });
}

void GLFWWebGPUBackend::signalFrameComplete() {
    frameInProgress = false;
    frameCV.notify_all();
}
