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
#import <CoreGraphics/CoreGraphics.h>
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
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_glfw.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <cstdlib>
#include <optional>
#include <iostream>
#include <limits>
#include <mutex>
#include <string>
#include <thread>

namespace {

void logUncapturedError(const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
    mbgl::Log::Error(mbgl::Event::Render,
                     std::string("Dawn validation error [") +
                         std::to_string(static_cast<int>(type)) + "] " +
                         (message.data ? std::string(message.data, message.length) : std::string()));
}

void logDeviceLost(const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
    mbgl::Log::Error(mbgl::Event::Render,
                     std::string("Dawn device lost [") +
                         std::to_string(static_cast<int>(reason)) + "] " +
                         (message.data ? std::string(message.data, message.length) : std::string()));
}

std::optional<WGPUBackendType> desiredBackendFromEnv() {
    if (const char* env = std::getenv("MLN_WEBGPU_BACKEND_TARGET")) {
        std::string value(env);
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
        if (value == "vulkan") {
            return WGPUBackendType_Vulkan;
        } else if (value == "opengl" || value == "gl" || value == "opengles" || value == "gles") {
            return WGPUBackendType_OpenGL;
        } else if (value == "metal") {
            return WGPUBackendType_Metal;
        } else if (value == "d3d11") {
            return WGPUBackendType_D3D11;
        } else if (value == "d3d12") {
            return WGPUBackendType_D3D12;
        }
    }
    return std::nullopt;
}

const char* backendTypeToString(WGPUBackendType type) {
    switch (type) {
        case WGPUBackendType_Null:
            return "Null";
        case WGPUBackendType_WebGPU:
            return "WebGPU";
        case WGPUBackendType_D3D11:
            return "D3D11";
        case WGPUBackendType_D3D12:
            return "D3D12";
        case WGPUBackendType_Metal:
            return "Metal";
        case WGPUBackendType_Vulkan:
            return "Vulkan";
        case WGPUBackendType_OpenGL:
            return "OpenGL";
        case WGPUBackendType_OpenGLES:
            return "OpenGLES";
        default:
            return "Unknown";
    }
}

bool backendMatches(WGPUBackendType candidate, WGPUBackendType desired) {
    if (desired == WGPUBackendType_OpenGL) {
        return candidate == WGPUBackendType_OpenGL || candidate == WGPUBackendType_OpenGLES;
    }
    return candidate == desired;
}

#ifdef __APPLE__
MTLPixelFormat toMetalPixelFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::BGRA8UnormSrgb:
            return MTLPixelFormatBGRA8Unorm_sRGB;
        case wgpu::TextureFormat::RGBA16Float:
            return MTLPixelFormatRGBA16Float;
        default:
            return MTLPixelFormatBGRA8Unorm;
    }
}
#endif

} // namespace

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

    WGPUTextureView getColorTextureView() override {
        if (auto* viewPtr = static_cast<WGPUTextureView>(backend.getCurrentTextureView())) {
            return viewPtr;
        }
        return nullptr;
    }

    WGPUTextureView getDepthStencilTextureView() override {
        if (auto* viewPtr = static_cast<WGPUTextureView>(backend.getDepthStencilView())) {
            return viewPtr;
        }
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

void GLFWWebGPUBackend::SpinLock::lock() {
    while (flag.test_and_set(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
}

void GLFWWebGPUBackend::SpinLock::unlock() {
    flag.clear(std::memory_order_release);
}

GLFWWebGPUBackend::GLFWWebGPUBackend(GLFWwindow* window_, bool capFrameRate)
    : GLFWBackend(),
      mbgl::webgpu::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::gfx::Renderable([window_] {
          int fbWidth = 0;
          int fbHeight = 0;
          if (window_) {
              glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
          }
          return mbgl::Size{static_cast<uint32_t>(std::max(fbWidth, 0)),
                            static_cast<uint32_t>(std::max(fbHeight, 0))};
      }(),
          std::make_unique<mbgl::WebGPURenderableResource>(*this)),
      window(window_) {
    static_cast<void>(capFrameRate);

    // Add small delay to let previous resources clean up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check window visibility
    if (window) {
        int visible = glfwGetWindowAttrib(window, GLFW_VISIBLE);

        if (!visible) {
            glfwShowWindow(window);
        }
    }


    // Create Dawn instance
    instance = std::make_unique<dawn::native::Instance>();

    // Enumerate adapters with retry logic
    std::vector<dawn::native::Adapter> adapters;
    for (int attempt = 0; attempt < 3; ++attempt) {
        adapters = instance->EnumerateAdapters();
        if (!adapters.empty()) {
            // Format the count as a string to avoid logging API issues
            std::string adapterMsg = "Found " + std::to_string(adapters.size()) + " Dawn adapters";
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (adapters.empty()) {
        throw std::runtime_error("No WebGPU adapters found after retries");
    }


    std::size_t selectedIndex = 0;
    if (auto targettedBackend = desiredBackendFromEnv()) {
        bool matched = false;
        for (std::size_t i = 0; i < adapters.size(); ++i) {
            WGPUAdapterInfo info = WGPU_ADAPTER_INFO_INIT;
            if (wgpuAdapterGetInfo(adapters[i].Get(), &info) != WGPUStatus_Success) {
                continue;
            }
            const auto candidate = info.backendType;
            const bool isMatch = backendMatches(candidate, *targettedBackend);
            wgpuAdapterInfoFreeMembers(info);
            if (isMatch) {
                selectedIndex = i;
                matched = true;
                mbgl::Log::Info(mbgl::Event::Render,
                                std::string("WebGPU: Forcing adapter backend to ") +
                                    backendTypeToString(candidate));
                break;
            }
        }
        if (!matched) {
            mbgl::Log::Warning(mbgl::Event::Render,
                               "WebGPU: Requested backend not available, using default adapter");
        }
    }

    dawn::native::Adapter& selectedAdapter = adapters[selectedIndex];

    {
        WGPUAdapterInfo info = WGPU_ADAPTER_INFO_INIT;
        if (wgpuAdapterGetInfo(selectedAdapter.Get(), &info) == WGPUStatus_Success) {
            mbgl::Log::Info(mbgl::Event::Render,
                            std::string("WebGPU: Selected adapter backend = ") +
                                backendTypeToString(info.backendType) +
                                ", name = " +
                                (info.device.data ? std::string(info.device.data, info.device.length)
                                                  : std::string("<unknown>")));
            wgpuAdapterInfoFreeMembers(info);
        }
    }

    std::vector<wgpu::FeatureName> supportedFeatures;
    WGPUSupportedFeatures features = WGPU_SUPPORTED_FEATURES_INIT;
    wgpuAdapterGetFeatures(selectedAdapter.Get(), &features);
    if (features.featureCount > 0 && features.features) {
        supportedFeatures.reserve(features.featureCount);
        for (size_t i = 0; i < features.featureCount; ++i) {
            supportedFeatures.push_back(static_cast<wgpu::FeatureName>(features.features[i]));
        }
        wgpuSupportedFeaturesFreeMembers(features);
    }
    const bool hasDepth32Stencil = std::find(supportedFeatures.begin(), supportedFeatures.end(),
                                             wgpu::FeatureName::Depth32FloatStencil8) != supportedFeatures.end();

    std::vector<wgpu::FeatureName> requiredFeatures;
    if (hasDepth32Stencil) {
        requiredFeatures.push_back(wgpu::FeatureName::Depth32FloatStencil8);
        depthStencilFormat = wgpu::TextureFormat::Depth32FloatStencil8;
    } else {
        depthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
    }


    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = "MapLibre WebGPU Device";
    deviceDesc.requiredFeatures = requiredFeatures.data();
    deviceDesc.requiredFeatureCount = requiredFeatures.size();
    deviceDesc.SetUncapturedErrorCallback(logUncapturedError);
    deviceDesc.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous, logDeviceLost);

    // Create device with descriptor
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

    setDepthStencilFormat(depthStencilFormat);

    // Process device events to ensure it's ready
    processEvents();

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
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    if (colorSpace) {
        layer.colorspace = colorSpace;
        CGColorSpaceRelease(colorSpace);
    }
    CGFloat scale = [nsWindow backingScaleFactor];
    if (scale <= 0.0) {
        scale = [[NSScreen mainScreen] backingScaleFactor];
    }
    layer.contentsScale = scale;

    // Get window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    layer.drawableSize = CGSizeMake(width, height);

    [view setLayer:layer];

    // Create surface from metal layer
    wgpu::SurfaceDescriptorFromMetalLayer metalDesc;
    metalDesc.layer = (__bridge void*)layer;

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &metalDesc;

    // Create surface using the instance
    wgpu::Instance wgpuInstance(instance->Get());
    wgpuSurface = wgpuInstance.CreateSurface(&surfaceDesc);

    if (wgpuSurface) {
        WGPUSurfaceCapabilities capabilities = WGPU_SURFACE_CAPABILITIES_INIT;
        if (wgpuSurfaceGetCapabilities(wgpuSurface.Get(), selectedAdapter.Get(), &capabilities) == WGPUStatus_Success) {
            const auto pickFormat = [&]() {
                const wgpu::TextureFormat preferredFormats[] = {
                    wgpu::TextureFormat::BGRA8Unorm,
                    wgpu::TextureFormat::BGRA8UnormSrgb,
                    wgpu::TextureFormat::RGBA8Unorm,
                    wgpu::TextureFormat::RGBA8UnormSrgb
                };
                for (const auto format : preferredFormats) {
                    for (size_t i = 0; i < capabilities.formatCount; ++i) {
                        if (capabilities.formats[i] == static_cast<WGPUTextureFormat>(format)) {
                            return format;
                        }
                    }
                }
                return capabilities.formatCount > 0 ? static_cast<wgpu::TextureFormat>(capabilities.formats[0])
                                                    : swapChainFormat;
            };
            swapChainFormat = pickFormat();
        }
        wgpuSurfaceCapabilitiesFreeMembers(capabilities);
    }

    setColorFormat(swapChainFormat);

    layer.pixelFormat = toMetalPixelFormat(swapChainFormat);

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
    configuredViewFormats[0] = swapChainFormat;
    config.viewFormatCount = 1;
    config.viewFormats = configuredViewFormats.data();

    wgpuSurface.Configure(&config);
    surfaceConfigured = true;
    lastConfiguredSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    createDepthStencilTexture(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
#elif defined(__linux__)
    // Get window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    std::string windowSizeMsg = "Window size: " + std::to_string(width) + "x" + std::to_string(height);

    // Create surface for WebGPU
    wgpu::Instance wgpuInstance(instance->Get());

    // Check if we can get X11 handles (through XWayland if on Wayland)
    Display* x11Display = glfwGetX11Display();
    Window x11Window = glfwGetX11Window(window);

    if (x11Display && x11Window) {

        wgpu::SurfaceDescriptorFromXlibWindow x11Desc = {};
        x11Desc.display = x11Display;
        x11Desc.window = x11Window;

        wgpu::SurfaceDescriptor surfaceDesc = {};
        surfaceDesc.nextInChain = &x11Desc;

        wgpuSurface = wgpuInstance.CreateSurface(&surfaceDesc);
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
        } else {
            throw std::runtime_error("Failed to get window surface from GLFW");
        }
    }

    if (wgpuSurface) {
        WGPUSurfaceCapabilities capabilities = WGPU_SURFACE_CAPABILITIES_INIT;
        if (wgpuSurfaceGetCapabilities(wgpuSurface.Get(), selectedAdapter.Get(), &capabilities) == WGPUStatus_Success) {
            const auto pickFormat = [&]() {
                const wgpu::TextureFormat preferredFormats[] = {
                    wgpu::TextureFormat::BGRA8Unorm,
                    wgpu::TextureFormat::BGRA8UnormSrgb,
                    wgpu::TextureFormat::RGBA8Unorm,
                    wgpu::TextureFormat::RGBA8UnormSrgb
                };
                for (const auto format : preferredFormats) {
                    for (size_t i = 0; i < capabilities.formatCount; ++i) {
                        if (capabilities.formats[i] == static_cast<WGPUTextureFormat>(format)) {
                            return format;
                        }
                    }
                }
                return capabilities.formatCount > 0 ? static_cast<wgpu::TextureFormat>(capabilities.formats[0])
                                                    : swapChainFormat;
            };
            swapChainFormat = pickFormat();
        }
        wgpuSurfaceCapabilitiesFreeMembers(capabilities);
    }

    setColorFormat(swapChainFormat);

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
    configuredViewFormats[0] = swapChainFormat;
    config.viewFormatCount = 1;
    config.viewFormats = configuredViewFormats.data();

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

    // Mark any in-flight frame as completed so tear-down logic can continue.
    frameInProgress.store(false, std::memory_order_release);

    {
        std::lock_guard<SpinLock> guard(textureStateLock);

        // Release texture views first (they reference textures)
        currentTextureView = nullptr;
        previousTextureView = nullptr;
        depthStencilView = nullptr;

        // Then release textures
        currentTexture = nullptr;
        previousTexture = nullptr;
        depthStencilTexture = nullptr;
    }

    // Ensure all GPU work is complete
    if (queue) {
        // Pump outstanding callbacks so pending GPU work can complete.
        processEvents();
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
#if defined(__APPLE__)
    @autoreleasepool {
#endif
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
        processEvents();
        // Only reconfigure if really needed (> 10 seconds)
        if (timeSinceLastSwap > 10000 && surfaceConfigured) {
            surfaceNeedsReconfigure = true;
        }
    }
    lastSwapTime = now;
    swapCount++;

    // Process any pending device work
    processEvents();

    // Run periodic maintenance
    periodicMaintenance();

    // Note: Command buffer is already submitted in CommandEncoder::present()
    // before swap() is called, so we don't need to submit it again here.

    // Wait for any previous frame to complete with longer timeout
    // if (!waitForFrame(std::chrono::milliseconds(500))) {
    //     // Timeout - force frame completion
    //     signalFrameComplete();
    // }

    // Present the current frame
    if (wgpuSurface && surfaceConfigured) {
        // Check if surface needs reconfiguration
        if (surfaceNeedsReconfigure) {
            reconfigureSurface();
        }

        // Only present if we have a valid texture view and haven't presented yet
        {
            std::lock_guard<SpinLock> guard(textureStateLock);

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
#if defined(__APPLE__)
    }
#endif
}

void GLFWWebGPUBackend::activate() {
    // WebGPU doesn't need explicit context activation like OpenGL
}

void GLFWWebGPUBackend::deactivate() {
    // WebGPU doesn't need explicit context deactivation like OpenGL
}

mbgl::Size GLFWWebGPUBackend::getSize() const {
    return size;
}

void GLFWWebGPUBackend::setSize(mbgl::Size newSize) {
    // Update swap chain size if needed
    if (size != newSize) {
#ifdef __APPLE__
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        if (static_cast<uint32_t>(width) != newSize.width ||
            static_cast<uint32_t>(height) != newSize.height) {
            NSWindow* nsWindow = glfwGetCocoaWindow(window);
            if (nsWindow) {
                CAMetalLayer* layer = (CAMetalLayer*)[[nsWindow contentView] layer];
                if (layer) {
                    layer.drawableSize = CGSizeMake(newSize.width, newSize.height);
                }
            }
        }
#endif
        size = newSize;
        surfaceNeedsReconfigure = true;
    }
}

void* GLFWWebGPUBackend::getCurrentTextureView() {
#if defined(__APPLE__)
    @autoreleasepool {
#endif
    // Don't do anything if we're shutting down
    if (isShuttingDown) {
        return nullptr;
    }

    if (!wgpuSurface || !surfaceConfigured) {
        return nullptr;
    }

    // Process any pending device work before acquiring texture
    processEvents();

    // Run periodic maintenance
    periodicMaintenance();

    // Check error threshold
    if (consecutiveErrors >= maxConsecutiveErrors) {
        // Too many errors, need to reconfigure
        surfaceNeedsReconfigure = true;
        reconfigureSurface();
        consecutiveErrors = 0;
    }

    std::unique_lock<SpinLock> lock(textureStateLock);

    if (currentTextureView && !framePresented) {
        return reinterpret_cast<void*>(currentTextureView.Get());
    }

    if (framePresented) {
        currentTextureView = nullptr;
        currentTexture = nullptr;
    }

    bool expected = false;
    if (!frameInProgress.compare_exchange_strong(expected, true)) {
        return nullptr;
    }

    framePresented = false;

    lock.unlock();

    wgpu::SurfaceTexture surfaceTexture;

    try {
        wgpuSurface.GetCurrentTexture(&surfaceTexture);
    } catch (...) {
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }

    if (!surfaceTexture.texture) {
        surfaceNeedsReconfigure = true;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }

    auto status = static_cast<wgpu::SurfaceGetCurrentTextureStatus>(surfaceTexture.status);
    if (status == wgpu::SurfaceGetCurrentTextureStatus::Outdated ||
        status == wgpu::SurfaceGetCurrentTextureStatus::Lost) {
        surfaceNeedsReconfigure = true;
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }

    if (status == wgpu::SurfaceGetCurrentTextureStatus::Timeout ||
        (status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
         status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)) {
        consecutiveErrors++;
        frameInProgress = false;
        return nullptr;
    }

    lock.lock();

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

#if defined(__APPLE__)
    }
#endif

    return reinterpret_cast<void*>(currentTextureView.Get());
}

mbgl::Size GLFWWebGPUBackend::getFramebufferSize() const {
    return getSize();
}

void* GLFWWebGPUBackend::getDepthStencilView() {
    std::lock_guard<SpinLock> guard(textureStateLock);
    if (!depthStencilView) {
        int width = 0;
        int height = 0;
        if (window) {
            glfwGetFramebufferSize(window, &width, &height);
        }
        createDepthStencilTexture(static_cast<uint32_t>(std::max(width, 0)),
                                  static_cast<uint32_t>(std::max(height, 0)));
    }
    return depthStencilView ? reinterpret_cast<void*>(depthStencilView.Get()) : nullptr;
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

    {
        std::lock_guard<SpinLock> guard(textureStateLock);

        // Clear all texture state
        currentTextureView = nullptr;
        currentTexture = nullptr;
        previousTextureView = nullptr;
        previousTexture = nullptr;
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

    size = {static_cast<uint32_t>(std::max(width, 0)), static_cast<uint32_t>(std::max(height, 0))};

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

    createDepthStencilTexture(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    // Update state
    surfaceConfigured = true;
    surfaceNeedsReconfigure = false;
    lastConfiguredSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    consecutiveErrors = 0;
}

void GLFWWebGPUBackend::processEvents() {
    // TODO(webgpu): hook Dawn event processing once the backend relies on it for
    // resource lifecycle management. For now, MapLibre doesn't depend on these
    // callbacks and invoking them triggers instability in the native backend.
}

void GLFWWebGPUBackend::createDepthStencilTexture(uint32_t width, uint32_t height) {
    depthStencilView = nullptr;
    depthStencilTexture = nullptr;

    if (!wgpuDevice || width == 0 || height == 0 || depthStencilFormat == wgpu::TextureFormat::Undefined) {
        return;
    }

    wgpu::TextureDescriptor depthDesc = {};
    depthDesc.label = "DepthStencilTexture";
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment;
    depthDesc.dimension = wgpu::TextureDimension::e2D;
    depthDesc.size = {width, height, 1};
    depthDesc.format = depthStencilFormat;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount = 1;

    depthStencilTexture = wgpuDevice.CreateTexture(&depthDesc);
    if (!depthStencilTexture) {
        mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: Failed to create depth/stencil texture");
        depthStencilFormat = wgpu::TextureFormat::Undefined;
        setDepthStencilFormat(depthStencilFormat);
        return;
    }

    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.label = "DepthStencilTextureView";
    viewDesc.format = depthDesc.format;
    viewDesc.dimension = wgpu::TextureViewDimension::e2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = wgpu::TextureAspect::All;

    depthStencilView = depthStencilTexture.CreateView(&viewDesc);
    if (!depthStencilView) {
        mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: Failed to create depth/stencil view");
        depthStencilTexture = nullptr;
        depthStencilFormat = wgpu::TextureFormat::Undefined;
        setDepthStencilFormat(depthStencilFormat);
    }
}

bool GLFWWebGPUBackend::waitForFrame(std::chrono::milliseconds timeout) {
    const auto start = std::chrono::steady_clock::now();

    while (frameInProgress.load(std::memory_order_acquire)) {
        if (isShuttingDown.load(std::memory_order_acquire)) {
            return true;
        }

        processEvents();

        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return true;
}

void GLFWWebGPUBackend::signalFrameComplete() {
    frameInProgress.store(false, std::memory_order_release);
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
        // Pump pending callbacks; we intentionally ignore errors here because
        // ProcessEvents returns false on device loss and we let the main loop
        // handle it on the next swap.
        processEvents();

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
};

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
        return;
    }

    ensureDebugTriangleResources();
    if (!debugTriangleInitialized || !debugTrianglePipeline || !debugTriangleVertexBuffer) {
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
}
