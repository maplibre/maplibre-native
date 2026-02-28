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
#include <cstring>

#if MLN_WEBGPU_IMPL_WGPU
#include <android/log.h>
#include <chrono>
#include <mutex>
#include <unistd.h>
#include <thread>

static void wgpuLogCallback(WGPULogLevel level, WGPUStringView message, void* /*userdata*/) {
    const int androidLevel = [&]() {
        switch (level) {
            case WGPULogLevel_Error: return ANDROID_LOG_ERROR;
            case WGPULogLevel_Warn: return ANDROID_LOG_WARN;
            case WGPULogLevel_Info: return ANDROID_LOG_INFO;
            case WGPULogLevel_Debug: return ANDROID_LOG_DEBUG;
            case WGPULogLevel_Trace: return ANDROID_LOG_VERBOSE;
            default: return ANDROID_LOG_INFO;
        }
    }();
    if (message.data && message.length > 0) {
        __android_log_print(androidLevel, "wgpu-native", "%.*s", static_cast<int>(message.length), message.data);
    }
}

// Redirect stderr to logcat so Rust panic messages are visible
static void startStderrToLogcat() {
    static std::once_flag flag;
    std::call_once(flag, [] {
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
    });
}
#endif

namespace mbgl {
namespace android {

class AndroidWebGPURenderableResource final : public webgpu::RenderableResource {
public:
    AndroidWebGPURenderableResource(AndroidWebGPURendererBackend& backend_)
        : backend(backend_) {}

    void bind() override { assert(false); }

    void swap() override { backend.markNeedsPresent(); }

    const mbgl::webgpu::RendererBackend& getBackend() const override { return backend; }

    const WGPUCommandEncoder& getCommandEncoder() const override {
        assert(false);
        static WGPUCommandEncoder dummy = nullptr;
        return dummy;
    }

    WGPURenderPassEncoder getRenderPassEncoder() const override {
        assert(false);
        return nullptr;
    }

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
    bool needsPresent = false;

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
    WGPUInstanceFeatureName instanceFeatures[] = {WGPUInstanceFeatureName_TimedWaitAny};
    WGPUInstanceLimits instanceLimits = WGPU_INSTANCE_LIMITS_INIT;
    instanceLimits.timedWaitAnyMaxCount = 64;

    WGPUInstanceDescriptor instanceDesc = WGPU_INSTANCE_DESCRIPTOR_INIT;
    instanceDesc.requiredFeatureCount = 1;
    instanceDesc.requiredFeatures = instanceFeatures;
    instanceDesc.requiredLimits = &instanceLimits;
    impl->instance = std::make_unique<dawn::native::Instance>(&instanceDesc);

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
    deviceDesc.SetUncapturedErrorCallback([](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
        Log::Error(Event::Render,
                   std::string("Dawn error [") + std::to_string(static_cast<int>(type)) + "] " +
                       (message.data ? std::string(message.data, message.length) : std::string()));
    });
    deviceDesc.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
            Log::Error(Event::Render,
                       std::string("Dawn device lost [") + std::to_string(static_cast<int>(reason)) + "] " +
                           (message.data ? std::string(message.data, message.length) : std::string()));
        });
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

    const int w = ANativeWindow_getWidth(window);
    const int h = ANativeWindow_getHeight(window);
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
    config.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
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
    if (!impl->currentTexture || !impl->device || !impl->queue || impl->framebufferSize.width == 0 ||
        impl->framebufferSize.height == 0) {
        return PremultipliedImage(impl->framebufferSize.isEmpty() ? Size(2, 2) : impl->framebufferSize);
    }

    const auto& fbSize = impl->framebufferSize;
    constexpr uint32_t bytesPerPixel = 4;
    constexpr uint32_t bytesPerRowAlignment = 256u;
    const uint32_t rowStride = fbSize.width * bytesPerPixel;
    const uint32_t alignedRowStride = ((rowStride + bytesPerRowAlignment - 1) / bytesPerRowAlignment) *
                                      bytesPerRowAlignment;
    const uint64_t alignedDataSize = static_cast<uint64_t>(alignedRowStride) * fbSize.height;

    auto data = std::make_unique<uint8_t[]>(static_cast<size_t>(rowStride) * fbSize.height);

#if MLN_WEBGPU_IMPL_DAWN
    auto device = impl->device.Get();
    auto queue = impl->queue.Get();
#elif MLN_WEBGPU_IMPL_WGPU
    auto device = static_cast<WGPUDevice>(impl->device);
    auto queue = static_cast<WGPUQueue>(impl->queue);
#endif

#if MLN_WEBGPU_IMPL_DAWN
    wgpuDeviceTick(device);
#endif

    WGPUBufferDescriptor bufferDesc = {};
#if MLN_WEBGPU_IMPL_DAWN
    WGPUStringView bufferLabel = {"Readback Buffer", strlen("Readback Buffer")};
    bufferDesc.label = bufferLabel;
#endif
    bufferDesc.size = alignedDataSize;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;

    WGPUBuffer stagingBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
    if (!stagingBuffer) {
        Log::Error(Event::Render, "WebGPU readFramebuffer: failed to create staging buffer");
        return {fbSize, std::move(data)};
    }

    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
    if (encoder) {
        WGPUTexelCopyTextureInfo src = {};
        src.texture = impl->currentTexture;
        src.mipLevel = 0;
        src.origin = {0, 0, 0};
        src.aspect = WGPUTextureAspect_All;

        WGPUTexelCopyBufferInfo dst = {};
        dst.buffer = stagingBuffer;
        dst.layout.offset = 0;
        dst.layout.bytesPerRow = alignedRowStride;
        dst.layout.rowsPerImage = fbSize.height;

        WGPUExtent3D copySize = {fbSize.width, fbSize.height, 1};
        wgpuCommandEncoderCopyTextureToBuffer(encoder, &src, &dst, &copySize);

        WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
        wgpuQueueSubmit(queue, 1, &commands);
        wgpuCommandBufferRelease(commands);
        wgpuCommandEncoderRelease(encoder);
    }

    struct MapContext {
        WGPUMapAsyncStatus status = WGPUMapAsyncStatus_Error;
        bool completed = false;
    };
    MapContext mapContext;

    WGPUBufferMapCallbackInfo callbackInfo = {};
    callbackInfo.callback = [](WGPUMapAsyncStatus status, WGPUStringView, void* userdata1, void*) {
        auto* ctx = static_cast<MapContext*>(userdata1);
        ctx->status = status;
        ctx->completed = true;
    };
    callbackInfo.userdata1 = &mapContext;

#if MLN_WEBGPU_IMPL_DAWN
    callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
    WGPUFuture future = wgpuBufferMapAsync(stagingBuffer, WGPUMapMode_Read, 0, alignedDataSize, callbackInfo);

    WGPUInstance instance = impl->instance->Get();
    if (instance) {
        WGPUFutureWaitInfo waitInfo = {};
        waitInfo.future = future;
        waitInfo.completed = false;

        WGPUWaitStatus waitStatus = wgpuInstanceWaitAny(instance, 1, &waitInfo, UINT64_MAX);
        if (waitStatus != WGPUWaitStatus_Success || !waitInfo.completed) {
            Log::Error(Event::Render, "WebGPU readFramebuffer: WaitAny failed");
        }
    }
#elif MLN_WEBGPU_IMPL_WGPU
    callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
    wgpuBufferMapAsync(stagingBuffer, WGPUMapMode_Read, 0, alignedDataSize, callbackInfo);

    for (int i = 0; i < 1000 && !mapContext.completed; ++i) {
        wgpuDevicePoll(device, true, nullptr);
        if (!mapContext.completed) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    if (!mapContext.completed) {
        Log::Error(Event::Render, "WebGPU readFramebuffer: buffer mapping timeout after polling");
    }
#endif

    if (mapContext.status == WGPUMapAsyncStatus_Success) {
        const auto* mappedData = static_cast<const uint8_t*>(
            wgpuBufferGetConstMappedRange(stagingBuffer, 0, alignedDataSize));
        if (mappedData) {
            const bool swizzle = (impl->surfaceFormat == WGPUTextureFormat_BGRA8Unorm);
            for (uint32_t row = 0; row < fbSize.height; ++row) {
                const auto* src = mappedData + static_cast<size_t>(row) * alignedRowStride;
                auto* dst = data.get() + static_cast<size_t>(row) * rowStride;
                if (swizzle) {
                    for (uint32_t x = 0; x < fbSize.width; ++x) {
                        dst[x * 4 + 0] = src[x * 4 + 2];
                        dst[x * 4 + 1] = src[x * 4 + 1];
                        dst[x * 4 + 2] = src[x * 4 + 0];
                        dst[x * 4 + 3] = src[x * 4 + 3];
                    }
                } else {
                    std::memcpy(dst, src, rowStride);
                }
            }
        }
    } else {
        Log::Error(Event::Render, "WebGPU readFramebuffer: buffer mapping failed");
    }

    wgpuBufferUnmap(stagingBuffer);
    wgpuBufferRelease(stagingBuffer);

    presentSurface();

    return {fbSize, std::move(data)};
}

void* AndroidWebGPURendererBackend::getCurrentTextureView() {
    if (!impl->surface || !impl->surfaceConfigured) {
        return nullptr;
    }

    presentSurface();

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

    // Actual texture size may differ from configured size (Vulkan currentExtent clamping)
    const uint32_t texW = wgpuTextureGetWidth(surfaceTexture.texture);
    const uint32_t texH = wgpuTextureGetHeight(surfaceTexture.texture);
    if (texW != size.width || texH != size.height) {
        if (texW > 0 && texH > 0) {
            size = {texW, texH};
            impl->framebufferSize = size;
            createDepthStencilTexture(texW, texH);
        }
    }

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

void AndroidWebGPURendererBackend::markNeedsPresent() {
    impl->needsPresent = true;
}

void AndroidWebGPURendererBackend::presentSurface() {
    if (!impl->surface || !impl->surfaceConfigured || !impl->needsPresent) {
        return;
    }

    impl->needsPresent = false;

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
