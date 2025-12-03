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
#import <Metal/Metal.h>

#if MLN_WEBGPU_IMPL_DAWN
#include <dawn/native/MetalBackend.h>
#endif

#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

#if MLN_WEBGPU_IMPL_DAWN
#include <dawn/native/VulkanBackend.h>
#endif

#endif

#ifdef None
#undef None
#endif

#if MLN_WEBGPU_IMPL_DAWN
#include <dawn/native/DawnNative.h>
#endif

#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

#if MLN_WEBGPU_IMPL_DAWN
#include <webgpu/webgpu_glfw.h>
#elif MLN_WEBGPU_IMPL_WGPU
#include <webgpu/wgpu.h>
#endif

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

// Helper macro for label assignment (wgpu-native needs WGPUStringView)
#if MLN_WEBGPU_IMPL_DAWN
// Dawn uses simple const char* labels - just assign directly inline
#define WGPU_LABEL(desc, labelStr) \
    (desc.label = labelStr)
#elif MLN_WEBGPU_IMPL_WGPU
#define WGPU_LABEL(desc, labelStr) \
    do { \
        static constexpr char label_text[] = labelStr; \
        WGPUStringView label_view = {label_text, sizeof(label_text) - 1}; \
        desc.label = label_view; \
    } while(0)

#endif

namespace {

#if MLN_WEBGPU_IMPL_DAWN
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

bool backendMatches(WGPUBackendType candidate, WGPUBackendType desired) {
    if (desired == WGPUBackendType_OpenGL) {
        return candidate == WGPUBackendType_OpenGL || candidate == WGPUBackendType_OpenGLES;
    }
    return candidate == desired;
}
#endif  // MLN_WEBGPU_IMPL_DAWN

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
      window(window_),
      enableVSync(capFrameRate) {

    // Add small delay to let previous resources clean up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check window visibility
    if (window) {
        int visible = glfwGetWindowAttrib(window, GLFW_VISIBLE);

        if (!visible) {
            glfwShowWindow(window);
        }
    }



#if MLN_WEBGPU_IMPL_DAWN

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

#elif MLN_WEBGPU_IMPL_WGPU
    // Create wgpu-native instance
    // Try without extras first to see if the instance can be created at all
    wgpuInstance = wgpu::createInstance();
    if (!wgpuInstance) {
        throw std::runtime_error("Failed to create WebGPU instance");
    }

    // Request adapter
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.powerPreference = wgpu::PowerPreference::HighPerformance;

    wgpu::Adapter selectedAdapter = wgpuInstance.requestAdapter(adapterOpts);
    if (!selectedAdapter) {
        throw std::runtime_error("No WebGPU adapter found");
    }

#endif


#if MLN_WEBGPU_IMPL_DAWN
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

#elif MLN_WEBGPU_IMPL_WGPU
    // Log adapter info for wgpu-native
    {
        WGPUAdapterInfo info = {};
        if (wgpuAdapterGetInfo(static_cast<WGPUAdapter>(selectedAdapter), &info) == WGPUStatus_Success) {
            mbgl::Log::Info(mbgl::Event::Render,
                            std::string("WebGPU: Selected adapter backend = ") +
                                backendTypeToString(info.backendType) +
                                ", name = " +
                                (info.device.data ? std::string(info.device.data, info.device.length)
                                                  : std::string("<unknown>")));
            wgpuAdapterInfoFreeMembers(info);
        }
    }
#endif

    std::vector<wgpu::FeatureName> supportedFeatures;

    WGPUSupportedFeatures features = {};

#if MLN_WEBGPU_IMPL_DAWN
    wgpuAdapterGetFeatures(selectedAdapter.Get(), &features);
#elif MLN_WEBGPU_IMPL_WGPU
    wgpuAdapterGetFeatures(static_cast<WGPUAdapter>(selectedAdapter), &features);
#endif

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
#if MLN_WEBGPU_IMPL_DAWN
    deviceDesc.requiredFeatures = requiredFeatures.data();
#elif MLN_WEBGPU_IMPL_WGPU
    deviceDesc.requiredFeatures = reinterpret_cast<const WGPUFeatureName*>(requiredFeatures.data());
#endif
    deviceDesc.requiredFeatureCount = requiredFeatures.size();

#if MLN_WEBGPU_IMPL_DAWN
    // Dawn supports error callbacks at device creation time
    deviceDesc.label = "MapLibre WebGPU Device";
    deviceDesc.SetUncapturedErrorCallback(logUncapturedError);
    deviceDesc.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous, logDeviceLost);
#elif MLN_WEBGPU_IMPL_WGPU
    // wgpu-native: Use label through descriptor, callbacks are set after device creation
    WGPU_LABEL(deviceDesc, "MapLibre WebGPU Device");
#endif

#if MLN_WEBGPU_IMPL_DAWN
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
#elif MLN_WEBGPU_IMPL_WGPU
    // Create device with wgpu-native
    wgpuDevice = selectedAdapter.requestDevice(deviceDesc);
    if (!wgpuDevice) {
        throw std::runtime_error("Failed to create WebGPU device");
    }
#endif

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

#if MLN_WEBGPU_IMPL_DAWN
    // For Dawn, use Dawn's native API to get Metal device
    layer.device = dawn::native::metal::GetMTLDevice(wgpuDevice.Get());
#elif MLN_WEBGPU_IMPL_WGPU
    // For wgpu-native, don't set the device - the surface configuration will handle it
    // Setting it to a different device could cause performance issues
#endif
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

#if MLN_WEBGPU_IMPL_DAWN
    // Create surface from metal layer
    wgpu::SurfaceSourceMetalLayer metalDesc;
    metalDesc.layer = (__bridge void*)layer;

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &metalDesc;
#elif MLN_WEBGPU_IMPL_WGPU
   wgpu::SurfaceSourceMetalLayer metalDesc = {};
    metalDesc.chain.sType = wgpu::SType::SurfaceSourceMetalLayer;
    metalDesc.chain.next = nullptr;
    metalDesc.layer = (__bridge void*)layer;

    wgpu::SurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&metalDesc);
#endif

#if MLN_WEBGPU_IMPL_DAWN
    // Create surface using the instance
    wgpu::Instance wgpuInstance(instance->Get());
    wgpuSurface = wgpuInstance.CreateSurface(&surfaceDesc);
#elif MLN_WEBGPU_IMPL_WGPU
    wgpuSurface = wgpuInstance.createSurface(surfaceDesc);
#endif

    if (wgpuSurface) {

        WGPUSurfaceCapabilities capabilities = {};

#if MLN_WEBGPU_IMPL_DAWN
        if (wgpuSurfaceGetCapabilities(wgpuSurface.Get(), selectedAdapter.Get(), &capabilities) == WGPUStatus_Success) {
#elif MLN_WEBGPU_IMPL_WGPU
        if (wgpuSurfaceGetCapabilities(static_cast<WGPUSurface>(wgpuSurface), static_cast<WGPUAdapter>(selectedAdapter), &capabilities) == WGPUStatus_Success) {
#endif
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
#if MLN_WEBGPU_IMPL_DAWN
    queue = wgpuDevice.GetQueue();
#elif MLN_WEBGPU_IMPL_WGPU
    queue = wgpuDevice.getQueue();
#endif

    // Configure surface
    wgpu::SurfaceConfiguration config = {};
    config.device = wgpuDevice;
    config.format = swapChainFormat;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;
    config.width = width;
    config.height = height;
    // Use Immediate mode (no vsync) in benchmark mode, Fifo (vsync) otherwise
    config.presentMode = enableVSync ? wgpu::PresentMode::Fifo : wgpu::PresentMode::Immediate;
    configuredViewFormats[0] = swapChainFormat;
    config.viewFormatCount = 1;

#if MLN_WEBGPU_IMPL_DAWN
    config.viewFormats = configuredViewFormats.data();

    wgpuSurface.Configure(&config);
#elif MLN_WEBGPU_IMPL_WGPU
    config.viewFormats = reinterpret_cast<const WGPUTextureFormat*>(configuredViewFormats.data());
    wgpuSurface.configure(config);
#endif

    surfaceConfigured = true;
    lastConfiguredSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    createDepthStencilTexture(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
#elif defined(__linux__)
    // Get window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    std::string windowSizeMsg = "Window size: " + std::to_string(width) + "x" + std::to_string(height);

    // Create surface for WebGPU
#if MLN_WEBGPU_IMPL_DAWN
    wgpu::Instance wgpuInstance(instance->Get());
#endif

    bool surfaceCreated = false;

#if defined(MLN_WITH_X11)
    if (!surfaceCreated) {
        Display* x11Display = glfwGetX11Display();
        Window x11Window = glfwGetX11Window(window);

        if (x11Display && x11Window) {
#if MLN_WEBGPU_IMPL_DAWN
            // Dawn uses a different structure without explicit chain member
            wgpu::SurfaceSourceXlibWindow x11Desc = {};
            x11Desc.display = x11Display;
            x11Desc.window = x11Window;

            wgpu::SurfaceDescriptor surfaceDesc = {};
            surfaceDesc.nextInChain = &x11Desc;

            wgpuSurface = wgpuInstance.CreateSurface(&surfaceDesc);
#elif MLN_WEBGPU_IMPL_WGPU
            // wgpu-native uses explicit chain members
            wgpu::SurfaceSourceXlibWindow x11Desc = {};
            x11Desc.chain.sType = wgpu::SType::SurfaceSourceXlibWindow;
            x11Desc.chain.next = nullptr;
            x11Desc.display = x11Display;
            x11Desc.window = x11Window;

            wgpu::SurfaceDescriptor surfaceDesc = {};
            surfaceDesc.nextInChain = reinterpret_cast<const WGPUChainedStruct*>(&x11Desc);

            wgpuSurface = wgpuInstance.createSurface(surfaceDesc);
#endif
            surfaceCreated = true;
        }
    }
#endif

#if defined(MLN_WITH_WAYLAND)
    if (!surfaceCreated) {
        struct wl_display* waylandDisplay = glfwGetWaylandDisplay();
        struct wl_surface* waylandSurface = glfwGetWaylandWindow(window);

        if (waylandDisplay && waylandSurface) {
            mbgl::Log::Warning(mbgl::Event::Render, "Attempting Wayland surface (experimental)");

            wgpu::SurfaceDescriptorFromWaylandSurface waylandDesc = {};
            waylandDesc.display = waylandDisplay;
            waylandDesc.surface = waylandSurface;

            wgpu::SurfaceDescriptor surfaceDesc = {};
            surfaceDesc.nextInChain = &waylandDesc;

#if MLN_WEBGPU_IMPL_DAWN
            wgpuSurface = wgpuInstance.CreateSurface(&surfaceDesc);
#elif MLN_WEBGPU_IMPL_WGPU
            wgpuSurface = wgpuInstance.createSurface(surfaceDesc);
#endif
            surfaceCreated = true;
        }
    }
#endif

    if (!surfaceCreated) {
#if defined(MLN_WITH_X11) && defined(MLN_WITH_WAYLAND)
        throw std::runtime_error("Failed to get X11 or Wayland window surface from GLFW");
#elif defined(MLN_WITH_X11)
        throw std::runtime_error("Failed to get X11 window surface from GLFW");
#elif defined(MLN_WITH_WAYLAND)
        throw std::runtime_error("Failed to get Wayland window surface from GLFW");
#else
        throw std::runtime_error("WebGPU backend built without window system support");
#endif
    }

    if (wgpuSurface) {

        WGPUSurfaceCapabilities capabilities = {};
#if MLN_WEBGPU_IMPL_DAWN
        if (wgpuSurfaceGetCapabilities(wgpuSurface.Get(), selectedAdapter.Get(), &capabilities) == WGPUStatus_Success) {
#elif MLN_WEBGPU_IMPL_WGPU
            if (wgpuSurfaceGetCapabilities(static_cast<WGPUSurface>(wgpuSurface), static_cast<WGPUAdapter>(selectedAdapter), &capabilities) == WGPUStatus_Success) {
#endif
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
#if MLN_WEBGPU_IMPL_DAWN
    queue = wgpuDevice.GetQueue();
#elif MLN_WEBGPU_IMPL_WGPU
    queue = wgpuDevice.getQueue();
#endif

    // Configure surface
    wgpu::SurfaceConfiguration config = {};
    config.device = wgpuDevice;
    config.format = swapChainFormat;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;
    config.width = width;
    config.height = height;
    // Use Immediate mode (no vsync) in benchmark mode, Fifo (vsync) otherwise
    config.presentMode = enableVSync ? wgpu::PresentMode::Fifo : wgpu::PresentMode::Immediate;
    configuredViewFormats[0] = swapChainFormat;
    config.viewFormatCount = 1;

#if MLN_WEBGPU_IMPL_DAWN
    config.viewFormats = configuredViewFormats.data();
    wgpuSurface.Configure(&config);
#elif MLN_WEBGPU_IMPL_WGPU
    config.viewFormats = reinterpret_cast<const WGPUTextureFormat*>(configuredViewFormats.data());
    wgpuSurface.configure(config);
#endif

    surfaceConfigured = true;
    lastConfiguredSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
#endif

    // Store WebGPU instance, device and queue in the base class

#if MLN_WEBGPU_IMPL_DAWN
    setInstance(instance->Get());
    setDevice(reinterpret_cast<void*>(wgpuDevice.Get()));
    setQueue(reinterpret_cast<void*>(queue.Get()));
#elif MLN_WEBGPU_IMPL_WGPU
    setInstance(static_cast<WGPUInstance>(wgpuInstance));
    setDevice(static_cast<WGPUDevice>(wgpuDevice));
    setQueue(static_cast<WGPUQueue>(queue));
#endif

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

#if MLN_WEBGPU_IMPL_DAWN
        wgpuSurface.Unconfigure();
#elif MLN_WEBGPU_IMPL_WGPU
        wgpuSurface.unconfigure();
#endif
        surfaceConfigured = false;
    }

    // Release WebGPU resources in reverse order of creation
    wgpuSurface = nullptr;  // Surface depends on device
    queue = nullptr;         // Queue is owned by device
    wgpuDevice = nullptr;    // Device depends on instance

#if MLN_WEBGPU_IMPL_DAWN
    // Release Dawn instance last (it owns the backend)
    instance.reset();
#elif MLN_WEBGPU_IMPL_WGPU
    wgpuInstance = nullptr;
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
#if defined(__APPLE__)
    @autoreleasepool {
#endif
    // Don't do anything if we're shutting down
    if (isShuttingDown) {
        return;
    }

    // Keep track of swap calls for debugging
    [[maybe_unused]] static int swapCount = 0;
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

#if MLN_WEBGPU_IMPL_DAWN
                wgpuSurface.Present();
#elif MLN_WEBGPU_IMPL_WGPU
                wgpuSurface.present();
#endif
                framePresented = true;

                // Release resources immediately (wgpu-native performs better with immediate release)
                currentTextureView = nullptr;
                currentTexture = nullptr;
                previousTextureView = nullptr;
                previousTexture = nullptr;
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

    // Check error threshold
    if (consecutiveErrors >= maxConsecutiveErrors) {
        // Too many errors, need to reconfigure
        surfaceNeedsReconfigure = true;
        reconfigureSurface();
        consecutiveErrors = 0;
    }

    std::unique_lock<SpinLock> lock(textureStateLock);

    if (currentTextureView && !framePresented) {

#if MLN_WEBGPU_IMPL_DAWN
        return reinterpret_cast<void*>(currentTextureView.Get());
#elif MLN_WEBGPU_IMPL_WGPU
        return reinterpret_cast<void*>(static_cast<WGPUTextureView>(currentTextureView));
#endif

    }

    // Always release textures before acquiring new ones to avoid "already acquired" error
    currentTextureView = nullptr;
    currentTexture = nullptr;

    framePresented = false;

    lock.unlock();

    wgpu::SurfaceTexture surfaceTexture;

    try {
#if MLN_WEBGPU_IMPL_DAWN
        wgpuSurface.GetCurrentTexture(&surfaceTexture);
#elif MLN_WEBGPU_IMPL_WGPU
        wgpuSurface.getCurrentTexture(&surfaceTexture);
#endif

    } catch (...) {
        consecutiveErrors++;
        return nullptr;
    }

    if (!surfaceTexture.texture) {
        surfaceNeedsReconfigure = true;
        consecutiveErrors++;
        return nullptr;
    }

    auto status = static_cast<wgpu::SurfaceGetCurrentTextureStatus>(surfaceTexture.status);
    if (status == wgpu::SurfaceGetCurrentTextureStatus::Outdated ||
        status == wgpu::SurfaceGetCurrentTextureStatus::Lost) {
        surfaceNeedsReconfigure = true;
        consecutiveErrors++;
        return nullptr;
    }

    if (status == wgpu::SurfaceGetCurrentTextureStatus::Timeout ||
        (status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
         status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)) {
        consecutiveErrors++;
        return nullptr;
    }

    lock.lock();

    // Create texture view with explicit descriptor
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.format = swapChainFormat;
#if MLN_WEBGPU_IMPL_DAWN
    viewDesc.dimension = wgpu::TextureViewDimension::e2D;
#elif MLN_WEBGPU_IMPL_WGPU
    viewDesc.dimension = wgpu::TextureViewDimension::_2D;
#endif

    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = wgpu::TextureAspect::All;
    WGPU_LABEL(viewDesc, "SwapChain TextureView");

    // Store the texture to keep it alive
    currentTexture = surfaceTexture.texture;

    // Create the texture view with validation
    wgpu::TextureView newView;
    try {
#if MLN_WEBGPU_IMPL_DAWN
        newView = currentTexture.CreateView(&viewDesc);
#elif MLN_WEBGPU_IMPL_WGPU
        newView = currentTexture.createView(viewDesc);
#endif
    } catch (...) {
        // Handle any exceptions from Dawn
        currentTexture = nullptr;
        consecutiveErrors++;
        return nullptr;
    }

    // Validate the new view before storing
    if (!newView) {
        currentTexture = nullptr;
        consecutiveErrors++;
        return nullptr;
    }

    // Store the validated view
    currentTextureView = newView;

    // Return the created texture view
    if (!currentTextureView) {
        currentTexture = nullptr;
        consecutiveErrors++;
        return nullptr;
    }

    // Success - reset error counter
    consecutiveErrors = 0;

#if defined(__APPLE__)
    }
#endif

#if MLN_WEBGPU_IMPL_DAWN
    return reinterpret_cast<void*>(currentTextureView.Get());
#elif MLN_WEBGPU_IMPL_WGPU
    return reinterpret_cast<void*>(static_cast<WGPUTextureView>(currentTextureView));
#endif
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

#if MLN_WEBGPU_IMPL_DAWN
    return depthStencilView ? reinterpret_cast<void*>(depthStencilView.Get()) : nullptr;
#elif MLN_WEBGPU_IMPL_WGPU
    return depthStencilView ? reinterpret_cast<void*>(static_cast<WGPUTextureView>(depthStencilView)) : nullptr;
#endif


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
    // Use Immediate mode (no vsync) in benchmark mode, Fifo (vsync) otherwise
    config.presentMode = enableVSync ? wgpu::PresentMode::Fifo : wgpu::PresentMode::Immediate;

#if MLN_WEBGPU_IMPL_DAWN
    wgpuSurface.Configure(&config);
#elif MLN_WEBGPU_IMPL_WGPU
    wgpuSurface.configure(config);
#endif

    createDepthStencilTexture(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    // Update state
    surfaceConfigured = true;
    surfaceNeedsReconfigure = false;
    lastConfiguredSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    consecutiveErrors = 0;
}

void GLFWWebGPUBackend::processEvents() {

#if MLN_WEBGPU_IMPL_DAWN
    // Dawn: Process device events for resource lifecycle management
    // This ticks the Dawn device to process pending callbacks and resource cleanup
    if (wgpuDevice) {
        wgpuDeviceTick(wgpuDevice.Get());
    }
#elif MLN_WEBGPU_IMPL_WGPU
    // wgpu-native: Use wgpuDevicePoll to process GPU work
    // This is the lightweight equivalent of Dawn's wgpuDeviceTick
    // wgpuDevicePoll with wait=false just processes pending GPU work without blocking
    if (wgpuDevice) {
        wgpuDevicePoll(static_cast<WGPUDevice>(wgpuDevice), false, nullptr);
    }
#endif

}

void GLFWWebGPUBackend::createDepthStencilTexture(uint32_t width, uint32_t height) {
    depthStencilView = nullptr;
    depthStencilTexture = nullptr;

    if (!wgpuDevice || width == 0 || height == 0 || depthStencilFormat == wgpu::TextureFormat::Undefined) {
        return;
    }

    wgpu::TextureDescriptor depthDesc = {};
    WGPU_LABEL(depthDesc, "DepthStencilTexture");
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment;

#if MLN_WEBGPU_IMPL_DAWN
    depthDesc.dimension = wgpu::TextureDimension::e2D;
#elif MLN_WEBGPU_IMPL_WGPU
    depthDesc.dimension = wgpu::TextureDimension::_2D;
#endif


    depthDesc.size = {width, height, 1};
    depthDesc.format = depthStencilFormat;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount = 1;

#if MLN_WEBGPU_IMPL_DAWN
    depthStencilTexture = wgpuDevice.CreateTexture(&depthDesc);
#elif MLN_WEBGPU_IMPL_WGPU
    depthStencilTexture = wgpuDevice.createTexture(depthDesc);
#endif

    if (!depthStencilTexture) {
        mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: Failed to create depth/stencil texture");
        depthStencilFormat = wgpu::TextureFormat::Undefined;
        setDepthStencilFormat(depthStencilFormat);
        return;
    }

    wgpu::TextureViewDescriptor viewDesc = {};
    WGPU_LABEL(viewDesc, "DepthStencilTextureView");
    viewDesc.format = depthDesc.format;

#if MLN_WEBGPU_IMPL_DAWN
    viewDesc.dimension = wgpu::TextureViewDimension::e2D;
#elif MLN_WEBGPU_IMPL_WGPU
    viewDesc.dimension = wgpu::TextureViewDimension::_2D;
#endif

    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = wgpu::TextureAspect::All;

#if MLN_WEBGPU_IMPL_DAWN
    depthStencilView = depthStencilTexture.CreateView(&viewDesc);
#elif MLN_WEBGPU_IMPL_WGPU
    depthStencilView = depthStencilTexture.createView(viewDesc);
#endif

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
