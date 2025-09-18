#include "glfw_webgpu_backend.hpp"

#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/gfx/renderable.hpp>

#include <GLFW/glfw3.h>

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#include <dawn/native/MetalBackend.h>
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>
#include <dawn/native/VulkanBackend.h>
#endif

#ifdef None
#undef None
#endif

#include <dawn/native/DawnNative.h>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_glfw.h>

#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <limits>

// Forward declaration
class GLFWWebGPUBackend;

namespace mbgl {

// WebGPU-specific RenderableResource implementation (similar to Metal's)
class WebGPURenderableResource final : public webgpu::RenderableResource {
public:
    WebGPURenderableResource(GLFWWebGPUBackend& backend_)
        : backend(backend_) {}

    void bind() override {
        // Similar to Metal - prepare for rendering
        // Nothing to do here for WebGPU as command encoder is managed by CommandEncoder class
    }

    void swap() override {
        // Similar to Metal - submit command buffer and present surface
        // The command buffer submission happens in CommandEncoder::present()
        // Here we just present the surface
        backend.swap();
    }

    const mbgl::webgpu::RendererBackend& getBackend() const override {
        return backend;
    }

    const WGPUCommandEncoder& getCommandEncoder() const override {
        // WebGPU's command encoder is managed by the CommandEncoder class
        static WGPUCommandEncoder dummy = nullptr;
        return dummy;
    }

    WGPURenderPassEncoder getRenderPassEncoder() const override {
        // Render pass encoder is managed by RenderPass class
        return nullptr;
    }

private:
    GLFWWebGPUBackend& backend;
};

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
      mbgl::gfx::Renderable(mbgl::Size{0, 0}, std::make_unique<mbgl::WebGPURenderableResource>(*this)),
      window(window_) {
    

    // Add small delay to let previous resources clean up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check window visibility
    if (window) {
        int visible = glfwGetWindowAttrib(window, GLFW_VISIBLE);
        mbgl::Log::Info(mbgl::Event::Render, "GLFW window visible: " + std::to_string(visible));

        if (!visible) {
            mbgl::Log::Info(mbgl::Event::Render, "Showing GLFW window...");
            glfwShowWindow(window);
        }
    }


    // Create Dawn instance
    mbgl::Log::Info(mbgl::Event::Render, "Initializing Dawn WebGPU backend...");
    instance = std::make_unique<dawn::native::Instance>();
    mbgl::Log::Info(mbgl::Event::Render, "Dawn instance created successfully");

    // Enumerate adapters with retry logic
    std::vector<dawn::native::Adapter> adapters;
    for (int attempt = 0; attempt < 3; ++attempt) {
        adapters = instance->EnumerateAdapters();
        if (!adapters.empty()) {
            // Format the count as a string to avoid logging API issues
            std::string adapterMsg = "Found " + std::to_string(adapters.size()) + " Dawn adapters";
            mbgl::Log::Info(mbgl::Event::Render, adapterMsg);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (adapters.empty()) {
        throw std::runtime_error("No WebGPU adapters found after retries");
    }

    // Log that we're using Dawn WebGPU implementation
    mbgl::Log::Info(mbgl::Event::Render, "Using Dawn WebGPU implementation with native backend");

    // Select first adapter (should be Vulkan on Linux)
    dawn::native::Adapter& selectedAdapter = adapters[0];
    mbgl::Log::Info(mbgl::Event::Render, "Selected Dawn adapter for WebGPU rendering");

    // Set up device descriptor with error callback
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = "MapLibre WebGPU Device";

    // Request features we need
    std::vector<wgpu::FeatureName> requiredFeatures;
    deviceDesc.requiredFeatures = requiredFeatures.data();
    deviceDesc.requiredFeatureCount = requiredFeatures.size();

    // Create device with descriptor
    mbgl::Log::Info(mbgl::Event::Render, "Creating Dawn WebGPU device...");
    WGPUDevice rawDevice = selectedAdapter.CreateDevice(&deviceDesc);
    if (!rawDevice) {
        // Retry once after delay
        mbgl::Log::Warning(mbgl::Event::Render, "Failed to create device, retrying...");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        rawDevice = selectedAdapter.CreateDevice(&deviceDesc);
        if (!rawDevice) {
            throw std::runtime_error("Failed to create WebGPU device after retry");
        }
    }

    wgpuDevice = wgpu::Device::Acquire(rawDevice);
    mbgl::Log::Info(mbgl::Event::Render, "Dawn WebGPU device created successfully");

    // Process device events to ensure it's ready
    wgpuDeviceTick(rawDevice);

    // Note: Dawn's error callback API is different from wgpu-native
    // Errors will be logged by Dawn's internal validation for now

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
#elif defined(__linux__)
    // Get window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    std::string windowSizeMsg = "Window size: " + std::to_string(width) + "x" + std::to_string(height);
    mbgl::Log::Info(mbgl::Event::Render, windowSizeMsg);

    // Create surface for WebGPU
    mbgl::Log::Info(mbgl::Event::Render, "Creating surface for WebGPU...");
    wgpu::Instance wgpuInstance(instance->Get());

    // Check if we can get X11 handles (through XWayland if on Wayland)
    Display* x11Display = glfwGetX11Display();
    Window x11Window = glfwGetX11Window(window);

    if (x11Display && x11Window) {
        mbgl::Log::Info(mbgl::Event::Render, "Using X11 surface (may be through XWayland)");

        wgpu::SurfaceDescriptorFromXlibWindow x11Desc = {};
        x11Desc.display = x11Display;
        x11Desc.window = x11Window;

        wgpu::SurfaceDescriptor surfaceDesc = {};
        surfaceDesc.nextInChain = &x11Desc;

        wgpuSurface = wgpuInstance.CreateSurface(&surfaceDesc);
        mbgl::Log::Info(mbgl::Event::Render, "Dawn WebGPU X11 surface created");
    } else {
        // Try Wayland even though Dawn may not fully support it
        struct wl_display* waylandDisplay = glfwGetWaylandDisplay();
        struct wl_surface* waylandSurface = glfwGetWaylandWindow(window);

        if (waylandDisplay && waylandSurface) {
            mbgl::Log::Warning(mbgl::Event::Render, "Attempting Wayland surface (experimental)");

            wgpu::SurfaceDescriptorFromWaylandSurface waylandDesc = {};
            waylandDesc.display = waylandDisplay;
            waylandDesc.surface = waylandSurface;

            wgpu::SurfaceDescriptor surfaceDesc = {};
            surfaceDesc.nextInChain = &waylandDesc;

            wgpuSurface = wgpuInstance.CreateSurface(&surfaceDesc);
            mbgl::Log::Info(mbgl::Event::Render, "Dawn WebGPU Wayland surface created");
        } else {
            throw std::runtime_error("Failed to get window surface from GLFW");
        }
    }

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
    mbgl::Log::Info(mbgl::Event::Render, "Dawn WebGPU surface configured successfully");
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

    // Note: Command buffer is already submitted in CommandEncoder::present()
    // before swap() is called, so we don't need to submit it again here.

    // Wait for any previous frame to complete with longer timeout
    if (!waitForFrame(std::chrono::milliseconds(500))) {
        // Timeout - force frame completion
        signalFrameComplete();
    }

    // Capture current texture view for optional debug overlay
    wgpu::TextureView viewForDebug;
    {
        std::lock_guard<std::mutex> lock(textureViewMutex);
        if (currentTextureView && !framePresented) {
            viewForDebug = currentTextureView;
        }
    }

    if (viewForDebug) {
        drawDebugTriangle(viewForDebug);
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
                wgpuSurface.Present();
                framePresented = true;

                // Move current resources to previous (keep them alive)
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
    
    // If we already have a current texture view for this frame, return it
    // This ensures we use the same texture view for all render passes in a single frame
    if (currentTextureView && !framePresented) {
        // Return the existing texture view for this frame
        return reinterpret_cast<void*>(currentTextureView.Get());
    }

    // If frame was already presented, we need a new texture for the next frame
    if (framePresented) {
        // Clear old texture view since we're starting a new frame
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

    static int getTextureCount = 0;
    if (getTextureCount++ < 20) {
        mbgl::Log::Info(mbgl::Event::Render, "Getting surface texture #" + std::to_string(getTextureCount));
    }

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

void* GLFWWebGPUBackend::getDepthStencilView() {
    // Return the depth/stencil view if we have one
    if (depthStencilView) {
        return reinterpret_cast<void*>(depthStencilView.Get());
    }

    // If we don't have one, try to create it
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    if (width > 0 && height > 0 && wgpuDevice) {
        // Create depth texture
        wgpu::TextureDescriptor depthDesc = {};
        depthDesc.usage = wgpu::TextureUsage::RenderAttachment;
        depthDesc.dimension = wgpu::TextureDimension::e2D;
        depthDesc.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        depthDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        depthDesc.mipLevelCount = 1;
        depthDesc.sampleCount = 1;
        depthDesc.label = "Depth/Stencil Texture";

        depthStencilTexture = wgpuDevice.CreateTexture(&depthDesc);

        if (depthStencilTexture) {
            wgpu::TextureViewDescriptor viewDesc = {};
            viewDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.baseMipLevel = 0;
            viewDesc.mipLevelCount = 1;
            viewDesc.baseArrayLayer = 0;
            viewDesc.arrayLayerCount = 1;
            viewDesc.aspect = wgpu::TextureAspect::All;
            viewDesc.label = "Depth/Stencil TextureView";

            depthStencilView = depthStencilTexture.CreateView(&viewDesc);
        }
    }

    return reinterpret_cast<void*>(depthStencilView.Get());
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
    
    // Create depth/stencil texture if needed or if size changed
    if (!depthStencilTexture ||
        depthStencilTexture.GetWidth() != static_cast<uint32_t>(width) ||
        depthStencilTexture.GetHeight() != static_cast<uint32_t>(height)) {

        // Release old depth texture
        depthStencilView = nullptr;
        depthStencilTexture = nullptr;

        // Create new depth texture
        wgpu::TextureDescriptor depthDesc = {};
        depthDesc.usage = wgpu::TextureUsage::RenderAttachment;
        depthDesc.dimension = wgpu::TextureDimension::e2D;
        depthDesc.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        depthDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        depthDesc.mipLevelCount = 1;
        depthDesc.sampleCount = 1;
        depthDesc.label = "Depth/Stencil Texture";

        depthStencilTexture = wgpuDevice.CreateTexture(&depthDesc);

        if (depthStencilTexture) {
            wgpu::TextureViewDescriptor viewDesc = {};
            viewDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.baseMipLevel = 0;
            viewDesc.mipLevelCount = 1;
            viewDesc.baseArrayLayer = 0;
            viewDesc.arrayLayerCount = 1;
            viewDesc.aspect = wgpu::TextureAspect::All;
            viewDesc.label = "Depth/Stencil TextureView";

            depthStencilView = depthStencilTexture.CreateView(&viewDesc);
        }
    }

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

void GLFWWebGPUBackend::ensureDebugTriangleResources() {
    if (debugTriangleInitialized || !wgpuDevice || !queue) {
        return;
    }

    static constexpr char shaderSource[] = R"(
struct VertexOut {
    @builtin(position) position : vec4<f32>,
    @location(0) color : vec3<f32>,
}

@vertex
fn vs_main(@location(0) position : vec2<f32>,
           @location(1) color : vec3<f32>) -> VertexOut {
    var out : VertexOut;
    out.position = vec4<f32>(position, 0.0, 1.0);
    out.color = color;
    return out;
}

@fragment
fn fs_main(input : VertexOut) -> @location(0) vec4<f32> {
    return vec4<f32>(input.color, 1.0);
}
)";

    wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
    wgslDesc.code = shaderSource;

    wgpu::ShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain = &wgslDesc;
    shaderDesc.label = "DebugTriangleShader";

    auto shaderModule = wgpuDevice.CreateShaderModule(&shaderDesc);
    if (!shaderModule) {
        return;
    }

    wgpu::VertexAttribute attributes[2];
    attributes[0].format = wgpu::VertexFormat::Float32x2;
    attributes[0].offset = 0;
    attributes[0].shaderLocation = 0;
    attributes[1].format = wgpu::VertexFormat::Float32x3;
    attributes[1].offset = sizeof(float) * 2;
    attributes[1].shaderLocation = 1;

    wgpu::VertexBufferLayout vertexLayout{};
    vertexLayout.arrayStride = sizeof(float) * 5;
    vertexLayout.attributeCount = 2;
    vertexLayout.attributes = attributes;
    vertexLayout.stepMode = wgpu::VertexStepMode::Vertex;

    wgpu::VertexState vertexState{};
    vertexState.module = shaderModule;
    vertexState.entryPoint = "vs_main";
    vertexState.bufferCount = 1;
    vertexState.buffers = &vertexLayout;

    wgpu::ColorTargetState colorTarget{};
    colorTarget.format = swapChainFormat;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    wgpu::PrimitiveState primitiveState{};
    primitiveState.topology = wgpu::PrimitiveTopology::TriangleList;
    primitiveState.stripIndexFormat = wgpu::IndexFormat::Undefined;
    primitiveState.frontFace = wgpu::FrontFace::CCW;
    primitiveState.cullMode = wgpu::CullMode::None;

    wgpu::MultisampleState multisample{};
    multisample.count = 1;
    multisample.mask = ~0u;
    multisample.alphaToCoverageEnabled = false;

    wgpu::RenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.label = "DebugTrianglePipeline";
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive = primitiveState;
    pipelineDesc.multisample = multisample;

    debugTrianglePipeline = wgpuDevice.CreateRenderPipeline(&pipelineDesc);
    if (!debugTrianglePipeline) {
        return;
    }

    const float vertices[] = {
         0.0f,  0.8f,  1.0f, 0.0f, 0.0f,
        -0.8f, -0.8f,  0.0f, 1.0f, 0.0f,
         0.8f, -0.8f,  0.0f, 0.0f, 1.0f,
    };

    debugTriangleVertexBufferSize = sizeof(vertices);

    wgpu::BufferDescriptor bufferDesc{};
    bufferDesc.label = "DebugTriangleVertexBuffer";
    bufferDesc.size = debugTriangleVertexBufferSize;
    bufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;

    debugTriangleVertexBuffer = wgpuDevice.CreateBuffer(&bufferDesc);
    if (!debugTriangleVertexBuffer) {
        debugTrianglePipeline = nullptr;
        debugTriangleVertexBufferSize = 0;
        return;
    }

    queue.WriteBuffer(debugTriangleVertexBuffer, 0, vertices, debugTriangleVertexBufferSize);

    debugTriangleInitialized = true;
}

void GLFWWebGPUBackend::drawDebugTriangle(const wgpu::TextureView& targetView) {
    if (!targetView) {
        mbgl::Log::Info(mbgl::Event::Render, "Debug triangle: no target view");
        return;
    }

    ensureDebugTriangleResources();
    if (!debugTriangleInitialized || !debugTrianglePipeline || !debugTriangleVertexBuffer) {
        mbgl::Log::Info(mbgl::Event::Render, "Debug triangle: resources not ready");
        return;
    }

    wgpu::CommandEncoderDescriptor encoderDesc{};
    encoderDesc.label = "DebugTriangleEncoder";
    auto encoder = wgpuDevice.CreateCommandEncoder(&encoderDesc);
    if (!encoder) {
        return;
    }

    wgpu::RenderPassColorAttachment colorAttachment{};
    colorAttachment.view = targetView;
    colorAttachment.loadOp = wgpu::LoadOp::Load;
    colorAttachment.storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassDescriptor passDesc{};
    passDesc.label = "DebugTrianglePass";
    passDesc.colorAttachmentCount = 1;
    passDesc.colorAttachments = &colorAttachment;

    auto pass = encoder.BeginRenderPass(&passDesc);
    pass.SetViewport(0.0f, 0.0f, static_cast<float>(lastConfiguredSize.width), static_cast<float>(lastConfiguredSize.height), 0.0f, 1.0f);
    pass.SetScissorRect(0, 0, lastConfiguredSize.width, lastConfiguredSize.height);
    pass.SetPipeline(debugTrianglePipeline);
    pass.SetVertexBuffer(0, debugTriangleVertexBuffer, 0, debugTriangleVertexBufferSize);
    pass.Draw(3);
    pass.End();

    auto commandBuffer = encoder.Finish();

    queue.Submit(1, &commandBuffer);
    mbgl::Log::Info(mbgl::Event::Render, "Debug triangle draw submitted");
}
