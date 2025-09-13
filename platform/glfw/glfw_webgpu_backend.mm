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
    metalLayer = (__bridge_retained void*)layer;  // ARC will handle the retain

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
    // Set shutdown flag atomically with memory barrier
    isShuttingDown.store(true, std::memory_order_release);
    
    // Signal any waiting threads to wake up
    {
        std::lock_guard<std::mutex> lock(frameMutex);
        frameInProgress = false;
    }
    frameCV.notify_all();
    
    // Give threads time to exit their wait loops
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Clear all Dawn/WebGPU resources in proper order
    {
        std::lock_guard<std::mutex> lock(textureViewMutex);
        
        // Release texture views first (they reference textures)
        currentTextureView = nullptr;
        previousTextureView = nullptr;
        
        // Then release textures
        currentTexture = nullptr;
        previousTexture = nullptr;
    }
    
    // Ensure all GPU work is complete
    if (queue) {
        // Note: Dawn doesn't have a Finish() method, but tick processes pending work
        if (wgpuDevice) {
            wgpuDeviceTick(wgpuDevice.Get());
        }
    }

    // Unconfigure surface before releasing device
    if (wgpuSurface && surfaceConfigured) {
        wgpuSurface.Unconfigure();
        surfaceConfigured = false;
    }

    // Release WebGPU resources in reverse order of creation
    wgpuSurface = nullptr;  // Surface depends on device
    queue = nullptr;         // Queue is owned by device
    wgpuDevice = nullptr;    // Device depends on instance

#ifdef __APPLE__
    // Release Metal layer after all WebGPU resources
    if (metalLayer) {
        CFRelease(metalLayer);  // Release the bridged retain
        metalLayer = nullptr;
    }
#endif

    // Release Dawn instance last (it owns the backend)
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
    // Don't do anything if we're shutting down
    if (isShuttingDown) {
        return;
    }
    
    // Keep track of swap calls for debugging
    static int swapCount = 0;
    static auto lastSwapTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastSwap = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSwapTime).count();
    
    // If it's been too long since last swap, tick the device to keep it alive
    if (timeSinceLastSwap > 1000) {  // 1 second
        // Keep Dawn device alive during idle periods
        if (wgpuDevice) {
            wgpuDeviceTick(wgpuDevice.Get());
        }
        // Only reconfigure if really needed (> 10 seconds)
        if (timeSinceLastSwap > 10000 && surfaceConfigured) {
            surfaceNeedsReconfigure = true;
        }
    }
    lastSwapTime = now;
    swapCount++;
    
    // Process any pending device work
    if (wgpuDevice) {
        wgpuDeviceTick(wgpuDevice.Get());
    }
    
    // Run periodic maintenance
    periodicMaintenance();
    
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
                // Present without pointer validation - Dawn handles this internally
                wgpuSurface.Present();
                framePresented = true;
                
                // Move current resources to previous (keep them alive)
                // They will be released on the next swap
                previousTextureView = std::move(currentTextureView);
                previousTexture = std::move(currentTexture);
                
                // Clear current references
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
    // Don't do anything if we're shutting down
    if (isShuttingDown) {
        return nullptr;
    }
    
    if (!wgpuSurface || !surfaceConfigured) {
        return nullptr;
    }
    
    // Process any pending device work before acquiring texture
    if (wgpuDevice) {
        wgpuDeviceTick(wgpuDevice.Get());
    }
    
    // Run periodic maintenance
    periodicMaintenance();
    
    // Check error threshold
    if (consecutiveErrors >= maxConsecutiveErrors) {
        // Too many errors, need to reconfigure
        surfaceNeedsReconfigure = true;
        reconfigureSurface();
        consecutiveErrors = 0;
    }

    // Lock to protect texture view access
    std::lock_guard<std::mutex> lock(textureViewMutex);
    
    // Track when we acquired the current texture view
    static auto textureAcquiredTime = std::chrono::steady_clock::now();
    
    // If we already have a current texture view and haven't presented, check if it's still fresh
    if (currentTextureView && !framePresented) {
        auto now = std::chrono::steady_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - textureAcquiredTime).count();
        
        // If texture view is too old (e.g., > 1 second), discard it
        if (age > 1000) {
            currentTextureView = nullptr;
            currentTexture = nullptr;
            frameInProgress = false;
            framePresented = true;
        } else {
            // Return the texture view - Dawn handles validation
            return reinterpret_cast<void*>(currentTextureView.Get());
            // Invalid, clear both texture and view
            currentTextureView = nullptr;
            currentTexture = nullptr;
        }
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

    // Check if texture is valid
    if (!surfaceTexture.texture) {
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
    
    // Update the acquisition time
    textureAcquiredTime = std::chrono::steady_clock::now();
    
    // Create the texture view with validation
    wgpu::TextureView newView;
    try {
        newView = currentTexture.CreateView(&viewDesc);
    } catch (...) {
        // Handle any exceptions from Dawn
        currentTexture = nullptr;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }
    
    // Validate the new view before storing
    if (!newView) {
        currentTexture = nullptr;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }
    
    // Store the validated view
    currentTextureView = newView;
    
    // Return the created texture view
    if (!currentTextureView) {
        currentTexture = nullptr;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }
    
    // Success - reset error counter
    consecutiveErrors = 0;
    
    return reinterpret_cast<void*>(currentTextureView.Get());
}

mbgl::Size GLFWWebGPUBackend::getFramebufferSize() const {
    return getSize();
}

void GLFWWebGPUBackend::reconfigureSurface() {
    if (!wgpuSurface || !wgpuDevice || isShuttingDown) {
        return;
    }
    
    // Wait for any in-progress frame
    waitForFrame();
    
    // Don't proceed if we're shutting down after waiting
    if (isShuttingDown) {
        return;
    }
    
    // Lock to ensure exclusive access
    std::lock_guard<std::mutex> lock(textureViewMutex);
    
    // Clear all texture state
    currentTextureView = nullptr;
    currentTexture = nullptr;
    previousTextureView = nullptr;
    previousTexture = nullptr;
    
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
    // Check shutdown flag with acquire ordering to see destructor's release
    if (isShuttingDown.load(std::memory_order_acquire)) {
        return true;
    }
    
    std::unique_lock<std::mutex> lock(frameMutex);
    if (!frameInProgress.load(std::memory_order_acquire)) {
        return true;  // No frame in progress
    }
    
    // Wait for frame to complete with timeout
    return frameCV.wait_for(lock, timeout, [this] { 
        return !frameInProgress.load(std::memory_order_acquire) || 
               isShuttingDown.load(std::memory_order_acquire); 
    });
}

void GLFWWebGPUBackend::signalFrameComplete() {
    // Don't use mutex if we're shutting down
    if (isShuttingDown) {
        frameInProgress = false;
        return;
    }
    
    frameInProgress = false;
    frameCV.notify_all();
}

void GLFWWebGPUBackend::periodicMaintenance() {
    // Check shutdown with proper memory ordering
    if (isShuttingDown.load(std::memory_order_acquire)) {
        return;
    }
    
    static auto lastMaintenanceTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastMaintenance = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMaintenanceTime).count();
    
    // Run maintenance every 100ms to keep Dawn alive during idle periods
    if (timeSinceLastMaintenance > 100) {
        // Tick the device to process any pending work and keep connection alive
        if (wgpuDevice) {
            try {
                wgpuDeviceTick(wgpuDevice.Get());
            } catch (...) {
                // Ignore exceptions during maintenance
            }
        }
        
        lastMaintenanceTime = now;
    }
}
