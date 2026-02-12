#include "android_webgpu_renderer_backend.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/webgpu/context.hpp>

#include <webgpu/webgpu.h>

#if MLN_WEBGPU_IMPL_DAWN
#include <dawn/native/DawnNative.h>
#elif MLN_WEBGPU_IMPL_WGPU
#include <wgpu.h>
#endif

#include <cassert>

#if MLN_WEBGPU_IMPL_WGPU
#include <android/log.h>
#include <unistd.h>
#include <thread>

static void wgpuLogCallback(WGPULogLevel level, WGPUStringView message, void* /*userdata*/) {
    const androidLevel = ANDROID_LOG_INFO;
    switch (level) {
        case WGPULogLevel_Error:
            androidLevel = ANDROID_LOG_ERROR;
            break;
        case WGPULogLevel_Warn:
            androidLevel = ANDROID_LOG_WARN;
            break;
        case WGPULogLevel_Info:
            androidLevel = ANDROID_LOG_INFO;
            break;
        case WGPULogLevel_Debug:
            androidLevel = ANDROID_LOG_DEBUG;
            break;
        case WGPULogLevel_Trace:
            androidLevel = ANDROID_LOG_VERBOSE;
            break;
        default:
            break;
    }
    if (message.data && message.length > 0) {
        __android_log_print(androidLevel, "wgpu-native", "%.*s", static_cast<int>(message.length), message.data);
    }
}

// Redirect stderr to logcat so Rust panic messages are visible
static void startStderrToLogcat() {
    int pipefd[2];
    if (pipe(pipefd) == -1) return;
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    std::thread([fd = pipefd[0]]() {
        char buf[1024];
        ssize_t n;
        while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
            buf[n] = '\0';
            __android_log_print(ANDROID_LOG_ERROR, "wgpu-native-stderr", "%s", buf);
        }
        close(fd);
    }).detach();
}
#endif

namespace mbgl {
namespace android {

class AndroidWebGPURenderableResource final : public webgpu::RenderableResource {
public:
    AndroidWebGPURenderableResource(AndroidWebGPURendererBackend& backend_)
        : backend(backend_) {}

    void bind() override {}

    void swap() override { backend.presentSurface(); }

    const mbgl::webgpu::RendererBackend& getBackend() const override { return backend; }

    const WGPUCommandEncoder& getCommandEncoder() const override {
        static WGPUCommandEncoder dummy = nullptr;
        return dummy;
    }

    WGPURenderPassEncoder getRenderPassEncoder() const override { return nullptr; }

    WGPUTextureView getColorTextureView() override {
        return static_cast<WGPUTextureView>(backend.getCurrentTextureView());
    }

    WGPUTextureView getDepthStencilTextureView() override {
        return static_cast<WGPUTextureView>(backend.getDepthStencilView());
    }

private:
    AndroidWebGPURendererBackend& backend;
};

class AndroidWebGPURendererBackend::Impl {
public:
#if MLN_WEBGPU_IMPL_DAWN
    std::unique_ptr<dawn::native::Instance> instance;
    WGPUAdapter adapter = nullptr;
#elif MLN_WEBGPU_IMPL_WGPU
    wgpu::Instance instance;
    wgpu::Adapter adapter;
#endif
    wgpu::Device device;
    wgpu::Queue queue;

    WGPUSurface surface = nullptr;
    WGPUTextureFormat surfaceFormat = WGPUTextureFormat_BGRA8Unorm;
    bool surfaceConfigured = false;

    WGPUTexture depthStencilTexture = nullptr;
    WGPUTextureView depthStencilView = nullptr;

    WGPUTexture currentTexture = nullptr;
    WGPUTextureView currentTextureView = nullptr;

    Size framebufferSize;
};

AndroidWebGPURendererBackend::AndroidWebGPURendererBackend(ANativeWindow* window_)
    : webgpu::RendererBackend(gfx::ContextMode::Unique),
      gfx::Renderable({64, 64}, std::make_unique<AndroidWebGPURenderableResource>(*this)),
      impl(std::make_unique<Impl>()),
      window(window_) {
#if MLN_WEBGPU_IMPL_WGPU
    startStderrToLogcat();
    wgpuSetLogLevel(WGPULogLevel_Warn);
    wgpuSetLogCallback(wgpuLogCallback, nullptr);
#endif

#if MLN_WEBGPU_IMPL_DAWN
    impl->instance = std::make_unique<dawn::native::Instance>();

    std::vector<dawn::native::Adapter> adapters = impl->instance->EnumerateAdapters();
    if (adapters.empty()) {
        Log::Error(Event::Render, "WebGPU: no adapters found");
        return;
    }

    size_t selectedIndex = 0;
    for (size_t i = 0; i < adapters.size(); ++i) {
        WGPUAdapterInfo info = WGPU_ADAPTER_INFO_INIT;
        if (wgpuAdapterGetInfo(adapters[i].Get(), &info) == WGPUStatus_Success) {
            if (info.backendType == WGPUBackendType_Vulkan) {
                selectedIndex = i;
                wgpuAdapterInfoFreeMembers(info);
                break;
            }
            wgpuAdapterInfoFreeMembers(info);
        }
    }

    dawn::native::Adapter& selectedAdapter = adapters[selectedIndex];

    WGPUSupportedFeatures features = {};
    wgpuAdapterGetFeatures(selectedAdapter.Get(), &features);
#elif MLN_WEBGPU_IMPL_WGPU
    wgpu::InstanceDescriptor instanceDesc = {};
    impl->instance = wgpu::createInstance(instanceDesc);

    if (!impl->instance) {
        Log::Error(Event::Render, "WebGPU: failed to create instance");
        return;
    }

    // Force Vulkan — the GL backend can't present to an ANativeWindow surface.
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.powerPreference = wgpu::PowerPreference::HighPerformance;
    adapterOpts.backendType = wgpu::BackendType::Vulkan;

    impl->adapter = impl->instance.requestAdapter(adapterOpts);
    if (!impl->adapter) {
        Log::Error(Event::Render, "WebGPU: no adapter found");
        return;
    }

    WGPUSupportedFeatures features = {};
    wgpuAdapterGetFeatures(static_cast<WGPUAdapter>(impl->adapter), &features);
#endif

    std::vector<wgpu::FeatureName> requiredFeatures;
    wgpu::TextureFormat depthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
    if (features.featureCount > 0 && features.features) {
        for (size_t i = 0; i < features.featureCount; ++i) {
            if (static_cast<wgpu::FeatureName>(features.features[i]) == wgpu::FeatureName::Depth32FloatStencil8) {
                requiredFeatures.push_back(wgpu::FeatureName::Depth32FloatStencil8);
                depthStencilFormat = wgpu::TextureFormat::Depth32FloatStencil8;
                break;
            }
        }
        wgpuSupportedFeaturesFreeMembers(features);
    }

    wgpu::DeviceDescriptor deviceDesc = {};
#if MLN_WEBGPU_IMPL_DAWN
    deviceDesc.label = "MapLibre Android WebGPU Device";
    deviceDesc.requiredFeatures = requiredFeatures.data();
#elif MLN_WEBGPU_IMPL_WGPU
    deviceDesc.requiredFeatures = reinterpret_cast<const WGPUFeatureName*>(requiredFeatures.data());
#endif
    deviceDesc.requiredFeatureCount = requiredFeatures.size();

#if MLN_WEBGPU_IMPL_DAWN
    WGPUDevice rawDevice = selectedAdapter.CreateDevice(&deviceDesc);
    if (!rawDevice) {
        Log::Error(Event::Render, "WebGPU: failed to create device");
        return;
    }

    impl->device = wgpu::Device::Acquire(rawDevice);
    impl->queue = impl->device.GetQueue();
    impl->adapter = selectedAdapter.Get();
    wgpuAdapterAddRef(impl->adapter);

    setDevice(impl->device.Get());
    setQueue(impl->queue.Get());
    setInstance(impl->instance->Get());
#elif MLN_WEBGPU_IMPL_WGPU
    impl->device = impl->adapter.requestDevice(deviceDesc);
    if (!impl->device) {
        Log::Error(Event::Render, "WebGPU: failed to create device");
        return;
    }

    impl->queue = impl->device.getQueue();

    setDevice(static_cast<WGPUDevice>(impl->device));
    setQueue(static_cast<WGPUQueue>(impl->queue));
    setInstance(static_cast<WGPUInstance>(impl->instance));
#endif
    setDepthStencilFormat(depthStencilFormat);

    createSurface();

    const w = ANativeWindow_getWidth(window);
    const h = ANativeWindow_getHeight(window);
    if (w > 0 && h > 0) {
        impl->framebufferSize = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
        size = impl->framebufferSize;
        configureSurface(impl->framebufferSize.width, impl->framebufferSize.height);
    }
}

AndroidWebGPURendererBackend::~AndroidWebGPURendererBackend() {
    if (impl) {
        if (impl->currentTextureView) {
            wgpuTextureViewRelease(impl->currentTextureView);
            impl->currentTextureView = nullptr;
        }
        if (impl->currentTexture) {
            wgpuTextureRelease(impl->currentTexture);
            impl->currentTexture = nullptr;
        }
        if (impl->depthStencilView) {
            wgpuTextureViewRelease(impl->depthStencilView);
            impl->depthStencilView = nullptr;
        }
        if (impl->depthStencilTexture) {
            wgpuTextureDestroy(impl->depthStencilTexture);
            wgpuTextureRelease(impl->depthStencilTexture);
            impl->depthStencilTexture = nullptr;
        }

        if (impl->surface) {
            if (impl->surfaceConfigured) {
                wgpuSurfaceUnconfigure(impl->surface);
            }
            wgpuSurfaceRelease(impl->surface);
            impl->surface = nullptr;
        }

        impl->queue = nullptr;
        impl->device = nullptr;
#if MLN_WEBGPU_IMPL_DAWN
        if (impl->adapter) {
            wgpuAdapterRelease(impl->adapter);
            impl->adapter = nullptr;
        }
        impl->instance.reset();
#elif MLN_WEBGPU_IMPL_WGPU
        impl->adapter = nullptr;
        impl->instance = nullptr;
#endif
    }
}

void AndroidWebGPURendererBackend::createSurface() {
#if MLN_WEBGPU_IMPL_DAWN
    if (!impl->instance || !window) return;

    WGPUSurfaceSourceAndroidNativeWindow androidDesc = WGPU_SURFACE_SOURCE_ANDROID_NATIVE_WINDOW_INIT;
    androidDesc.window = window;

    WGPUSurfaceDescriptor surfaceDesc = WGPU_SURFACE_DESCRIPTOR_INIT;
    surfaceDesc.nextInChain = &androidDesc.chain;

    WGPUInstance rawInstance = impl->instance->Get();
    impl->surface = wgpuInstanceCreateSurface(rawInstance, &surfaceDesc);
#elif MLN_WEBGPU_IMPL_WGPU
    if (!impl->instance || !window) return;

    WGPUSurfaceSourceAndroidNativeWindow androidDesc = {};
    androidDesc.chain.sType = WGPUSType_SurfaceSourceAndroidNativeWindow;
    androidDesc.chain.next = nullptr;
    androidDesc.window = window;

    WGPUSurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = &androidDesc.chain;

    impl->surface = wgpuInstanceCreateSurface(static_cast<WGPUInstance>(impl->instance), &surfaceDesc);
#endif

    if (!impl->surface) {
        Log::Error(Event::Render, "WebGPU: failed to create surface");
    }
}

void AndroidWebGPURendererBackend::configureSurface(uint32_t width, uint32_t height) {
    if (!impl->surface || !impl->device || width == 0 || height == 0) return;

    WGPUSurfaceCapabilities capabilities = {};
#if MLN_WEBGPU_IMPL_DAWN
    WGPUStatus capStatus = wgpuSurfaceGetCapabilities(impl->surface, impl->adapter, &capabilities);
#elif MLN_WEBGPU_IMPL_WGPU
    WGPUStatus capStatus = wgpuSurfaceGetCapabilities(
        impl->surface, static_cast<WGPUAdapter>(impl->adapter), &capabilities);
#endif

    WGPUCompositeAlphaMode selectedAlphaMode = WGPUCompositeAlphaMode_Opaque;
    WGPUPresentMode selectedPresentMode = WGPUPresentMode_Fifo;

    if (capStatus == WGPUStatus_Success) {
        const WGPUTextureFormat preferredFormats[] = {
            WGPUTextureFormat_BGRA8Unorm,
            WGPUTextureFormat_RGBA8Unorm,
        };
        for (auto preferred : preferredFormats) {
            for (size_t i = 0; i < capabilities.formatCount; ++i) {
                if (capabilities.formats[i] == preferred) {
                    impl->surfaceFormat = preferred;
                    goto formatFound;
                }
            }
        }
        if (capabilities.formatCount > 0) {
            impl->surfaceFormat = capabilities.formats[0];
        }
    formatFound:

        if (capabilities.alphaModeCount > 0) {
            selectedAlphaMode = capabilities.alphaModes[0];
        }
        if (capabilities.presentModeCount > 0) {
            selectedPresentMode = capabilities.presentModes[0];
        }

        wgpuSurfaceCapabilitiesFreeMembers(capabilities);
    }

    setColorFormat(static_cast<wgpu::TextureFormat>(impl->surfaceFormat));

#if MLN_WEBGPU_IMPL_DAWN
    WGPUSurfaceConfiguration config = WGPU_SURFACE_CONFIGURATION_INIT;
    config.device = impl->device.Get();
#elif MLN_WEBGPU_IMPL_WGPU
    WGPUSurfaceConfiguration config = {};
    config.device = static_cast<WGPUDevice>(impl->device);
#endif
    config.format = impl->surfaceFormat;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.alphaMode = selectedAlphaMode;
    config.width = width;
    config.height = height;
    config.presentMode = selectedPresentMode;

    wgpuSurfaceConfigure(impl->surface, &config);
    impl->surfaceConfigured = true;

    createDepthStencilTexture(width, height);
}

void AndroidWebGPURendererBackend::createDepthStencilTexture(uint32_t width, uint32_t height) {
    if (impl->depthStencilView) {
        wgpuTextureViewRelease(impl->depthStencilView);
        impl->depthStencilView = nullptr;
    }
    if (impl->depthStencilTexture) {
        wgpuTextureDestroy(impl->depthStencilTexture);
        wgpuTextureRelease(impl->depthStencilTexture);
        impl->depthStencilTexture = nullptr;
    }

    if (!impl->device || width == 0 || height == 0) return;

    WGPUTextureFormat dsFormat = static_cast<WGPUTextureFormat>(getDepthStencilFormat());

#if MLN_WEBGPU_IMPL_DAWN
    WGPUTextureDescriptor depthDesc = WGPU_TEXTURE_DESCRIPTOR_INIT;
#elif MLN_WEBGPU_IMPL_WGPU
    WGPUTextureDescriptor depthDesc = {};
#endif
    depthDesc.usage = WGPUTextureUsage_RenderAttachment;
    depthDesc.dimension = WGPUTextureDimension_2D;
    depthDesc.size = {width, height, 1};
    depthDesc.format = dsFormat;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount = 1;

#if MLN_WEBGPU_IMPL_DAWN
    impl->depthStencilTexture = wgpuDeviceCreateTexture(impl->device.Get(), &depthDesc);
#elif MLN_WEBGPU_IMPL_WGPU
    impl->depthStencilTexture = wgpuDeviceCreateTexture(static_cast<WGPUDevice>(impl->device), &depthDesc);
#endif
    if (!impl->depthStencilTexture) {
        Log::Warning(Event::Render, "WebGPU: failed to create depth/stencil texture");
        return;
    }

#if MLN_WEBGPU_IMPL_DAWN
    WGPUTextureViewDescriptor viewDesc = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
#elif MLN_WEBGPU_IMPL_WGPU
    WGPUTextureViewDescriptor viewDesc = {};
#endif
    viewDesc.format = dsFormat;
    viewDesc.dimension = WGPUTextureViewDimension_2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = WGPUTextureAspect_All;

    impl->depthStencilView = wgpuTextureCreateView(impl->depthStencilTexture, &viewDesc);
    if (!impl->depthStencilView) {
        Log::Warning(Event::Render, "WebGPU: failed to create depth/stencil view");
    }
}

void AndroidWebGPURendererBackend::resizeFramebuffer(int width, int height) {
    if (width <= 0 || height <= 0) return;

    impl->framebufferSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    size = impl->framebufferSize;

    if (impl->currentTextureView) {
        wgpuTextureViewRelease(impl->currentTextureView);
        impl->currentTextureView = nullptr;
    }
    if (impl->currentTexture) {
        wgpuTextureRelease(impl->currentTexture);
        impl->currentTexture = nullptr;
    }

    configureSurface(impl->framebufferSize.width, impl->framebufferSize.height);
}

PremultipliedImage AndroidWebGPURendererBackend::readFramebuffer() {
    // TODO not implemented
    return PremultipliedImage(Size(2, 2));
}

void* AndroidWebGPURendererBackend::getCurrentTextureView() {
    if (!impl->surface || !impl->surfaceConfigured) {
        return nullptr;
    }

    if (impl->currentTextureView) {
        wgpuTextureViewRelease(impl->currentTextureView);
        impl->currentTextureView = nullptr;
    }
    if (impl->currentTexture) {
        wgpuTextureRelease(impl->currentTexture);
        impl->currentTexture = nullptr;
    }

    WGPUSurfaceTexture surfaceTexture = {};
    wgpuSurfaceGetCurrentTexture(impl->surface, &surfaceTexture);

    if (!surfaceTexture.texture) {
        return nullptr;
    }

    auto status = static_cast<WGPUSurfaceGetCurrentTextureStatus>(surfaceTexture.status);
    if (status == WGPUSurfaceGetCurrentTextureStatus_Outdated || status == WGPUSurfaceGetCurrentTextureStatus_Lost) {
        wgpuTextureRelease(surfaceTexture.texture);
        configureSurface(impl->framebufferSize.width, impl->framebufferSize.height);
        return nullptr;
    }

    if (status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
        wgpuTextureRelease(surfaceTexture.texture);
        return nullptr;
    }

    impl->currentTexture = surfaceTexture.texture;

#if MLN_WEBGPU_IMPL_DAWN
    WGPUTextureViewDescriptor viewDesc = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
#elif MLN_WEBGPU_IMPL_WGPU
    WGPUTextureViewDescriptor viewDesc = {};
#endif
    viewDesc.format = impl->surfaceFormat;
    viewDesc.dimension = WGPUTextureViewDimension_2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = WGPUTextureAspect_All;

    impl->currentTextureView = wgpuTextureCreateView(impl->currentTexture, &viewDesc);

    return impl->currentTextureView;
}

void AndroidWebGPURendererBackend::presentSurface() {
    if (!impl->surface || !impl->surfaceConfigured) {
        return;
    }

    if (impl->currentTextureView) {
        wgpuSurfacePresent(impl->surface);
        wgpuTextureViewRelease(impl->currentTextureView);
        impl->currentTextureView = nullptr;
    }
    if (impl->currentTexture) {
        wgpuTextureRelease(impl->currentTexture);
        impl->currentTexture = nullptr;
    }

    if (impl->device) {
#if MLN_WEBGPU_IMPL_DAWN
        wgpuDeviceTick(impl->device.Get());
#elif MLN_WEBGPU_IMPL_WGPU
        wgpuDevicePoll(static_cast<WGPUDevice>(impl->device), false, nullptr);
#endif
    }
}

void* AndroidWebGPURendererBackend::getDepthStencilView() {
    return impl->depthStencilView;
}

mbgl::Size AndroidWebGPURendererBackend::getFramebufferSize() const {
    return impl->framebufferSize;
}

} // namespace android
} // namespace mbgl

namespace mbgl {
namespace gfx {

template <>
std::unique_ptr<android::AndroidRendererBackend> Backend::Create<mbgl::gfx::Backend::Type::WebGPU>(
    ANativeWindow* window) {
    return std::make_unique<android::AndroidWebGPURendererBackend>(window);
}

} // namespace gfx
} // namespace mbgl
