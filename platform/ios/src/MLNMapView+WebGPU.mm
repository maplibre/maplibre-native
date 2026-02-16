#import "MLNDisplayUtils.h"
#import "MLNFoundation_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNMapView+WebGPU.h"

#import <mbgl/webgpu/renderable_resource.hpp>
#import <mbgl/util/logging.hpp>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <webgpu/webgpu.h>

#if MLN_WEBGPU_IMPL_DAWN
#include <dawn/native/DawnNative.h>
#include <dawn/native/MetalBackend.h>
#include <dawn/dawn_proc.h>
#include <webgpu/webgpu_cpp.h>
#elif MLN_WEBGPU_IMPL_WGPU
#include <wgpu.h>
#include <webgpu/webgpu_cpp.h>
#endif

#include <cassert>

@interface MLNWebGPUView : UIView
@end

@implementation MLNWebGPUView
+ (Class)layerClass {
    return [CAMetalLayer class];
}
@end

class MLNMapViewWebGPURenderableResource final : public mbgl::webgpu::RenderableResource {
public:
    MLNMapViewWebGPURenderableResource(MLNMapViewWebGPUImpl& backend_)
        : backend(backend_) {}

    void bind() override {}

    void swap() override {
        backend.presentSurface();
    }

    const mbgl::webgpu::RendererBackend& getBackend() const override {
        return backend;
    }

    const WGPUCommandEncoder& getCommandEncoder() const override {
        static WGPUCommandEncoder dummy = nullptr;
        return dummy;
    }

    WGPURenderPassEncoder getRenderPassEncoder() const override {
        return nullptr;
    }

    WGPUTextureView getColorTextureView() override {
        return static_cast<WGPUTextureView>(backend.getCurrentTextureView());
    }

    WGPUTextureView getDepthStencilTextureView() override {
        return static_cast<WGPUTextureView>(backend.getDepthStencilView());
    }

private:
    MLNMapViewWebGPUImpl& backend;
};

class MLNMapViewWebGPUImpl::Impl {
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

    mbgl::Size framebufferSize;

    MLNWebGPUView* webGPUView = nil;
    bool presentsWithTransaction = false;
    NSUInteger activationCount = 0;
};

MLNMapViewWebGPUImpl::MLNMapViewWebGPUImpl(MLNMapView* nativeView_)
    : MLNMapViewImpl(nativeView_),
      mbgl::webgpu::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::gfx::Renderable({0, 0}, std::make_unique<MLNMapViewWebGPURenderableResource>(*this)),
      impl(std::make_unique<Impl>()) {

#if MLN_WEBGPU_IMPL_DAWN
    // Required when not using the monolithic Dawn library.
    dawnProcSetProcs(&dawn::native::GetProcs());

    impl->instance = std::make_unique<dawn::native::Instance>();

    wgpu::RequestAdapterOptions metalOpts = {};
    metalOpts.backendType = wgpu::BackendType::Metal;
    std::vector<dawn::native::Adapter> adapters = impl->instance->EnumerateAdapters(&metalOpts);
    if (adapters.empty()) {
        adapters = impl->instance->EnumerateAdapters();
    }
    if (adapters.empty()) {
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU iOS: No adapters found");
        return;
    }

    dawn::native::Adapter& selectedAdapter = adapters[0];

    bool hasDepth32FloatStencil8 = false;
    WGPUSupportedFeatures features = {};
    wgpuAdapterGetFeatures(selectedAdapter.Get(), &features);
    if (features.featureCount > 0 && features.features) {
        for (size_t i = 0; i < features.featureCount; ++i) {
            if (static_cast<wgpu::FeatureName>(features.features[i]) == wgpu::FeatureName::Depth32FloatStencil8) {
                hasDepth32FloatStencil8 = true;
                break;
            }
        }
        wgpuSupportedFeaturesFreeMembers(features);
    }
#elif MLN_WEBGPU_IMPL_WGPU
    wgpu::InstanceDescriptor instanceDesc = {};
    impl->instance = wgpu::createInstance(instanceDesc);
    if (!impl->instance) {
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU iOS: Failed to create instance");
        return;
    }

    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.powerPreference = wgpu::PowerPreference::HighPerformance;
    adapterOpts.backendType = wgpu::BackendType::Metal;

    impl->adapter = impl->instance.requestAdapter(adapterOpts);
    if (!impl->adapter) {
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU iOS: No adapter found");
        return;
    }

    WGPUSupportedFeatures features = {};
    bool hasDepth32FloatStencil8 = false;
    wgpuAdapterGetFeatures(static_cast<WGPUAdapter>(impl->adapter), &features);
    if (features.featureCount > 0 && features.features) {
        for (size_t i = 0; i < features.featureCount; ++i) {
            if (static_cast<wgpu::FeatureName>(features.features[i]) == wgpu::FeatureName::Depth32FloatStencil8) {
                hasDepth32FloatStencil8 = true;
                break;
            }
        }
        wgpuSupportedFeaturesFreeMembers(features);
    }
#endif

    std::vector<wgpu::FeatureName> requiredFeatures;
    wgpu::TextureFormat depthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
    if (hasDepth32FloatStencil8) {
        requiredFeatures.push_back(wgpu::FeatureName::Depth32FloatStencil8);
        depthStencilFormat = wgpu::TextureFormat::Depth32FloatStencil8;
    }

    wgpu::DeviceDescriptor deviceDesc = {};
#if MLN_WEBGPU_IMPL_DAWN
    deviceDesc.label = "MapLibre iOS WebGPU Device";
    deviceDesc.SetUncapturedErrorCallback([](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
        NSLog(@"[WebGPU] Device error (type=%d): %.*s", (int)type, (int)message.length, message.data);
    });
    deviceDesc.requiredFeatures = requiredFeatures.data();
#elif MLN_WEBGPU_IMPL_WGPU
    deviceDesc.requiredFeatures = reinterpret_cast<const WGPUFeatureName*>(requiredFeatures.data());
#endif
    deviceDesc.requiredFeatureCount = requiredFeatures.size();

#if MLN_WEBGPU_IMPL_DAWN
    WGPUDevice rawDevice = selectedAdapter.CreateDevice(&deviceDesc);
    if (!rawDevice) {
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU iOS: Failed to create device");
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
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU iOS: Failed to create device");
        return;
    }

    impl->queue = impl->device.getQueue();

    setDevice(static_cast<WGPUDevice>(impl->device));
    setQueue(static_cast<WGPUQueue>(impl->queue));
    setInstance(static_cast<WGPUInstance>(impl->instance));
#endif
    setDepthStencilFormat(depthStencilFormat);
}

MLNMapViewWebGPUImpl::~MLNMapViewWebGPUImpl() {
    if (!impl) return;

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

void MLNMapViewWebGPUImpl::createView() {
    if (impl->webGPUView) {
        return;
    }

    const auto scaleFactor = MLNEffectiveScaleFactorForView(mapView);

    impl->webGPUView = [[MLNWebGPUView alloc] initWithFrame:mapView.bounds];
    impl->webGPUView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    impl->webGPUView.contentScaleFactor = scaleFactor;
    impl->webGPUView.contentMode = UIViewContentModeCenter;
    impl->webGPUView.opaque = mapView.opaque;
    impl->webGPUView.layer.opaque = mapView.opaque;
    impl->webGPUView.userInteractionEnabled = NO;

    CAMetalLayer* metalLayer = (CAMetalLayer*)impl->webGPUView.layer;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.framebufferOnly = YES;

    CGSize drawableSize = CGSizeMake(mapView.bounds.size.width * scaleFactor,
                                     mapView.bounds.size.height * scaleFactor);
    metalLayer.drawableSize = drawableSize;

#if MLN_WEBGPU_IMPL_DAWN
    // Dawn needs the Metal device set on the layer before creating a surface.
    id<MTLDevice> mtlDevice = dawn::native::metal::GetMTLDevice(impl->device.Get());
    metalLayer.device = mtlDevice;
#endif

    if (@available(iOS 13.0, *)) {
        metalLayer.presentsWithTransaction = impl->presentsWithTransaction;
    }

    [mapView insertSubview:impl->webGPUView atIndex:0];

    createSurface();

    uint32_t w = static_cast<uint32_t>(drawableSize.width);
    uint32_t h = static_cast<uint32_t>(drawableSize.height);
    if (w > 0 && h > 0) {
        impl->framebufferSize = {w, h};
        size = impl->framebufferSize;
        configureSurface(w, h);
    }
}

void MLNMapViewWebGPUImpl::createSurface() {
    if (!impl->webGPUView) return;

    CAMetalLayer* metalLayer = (CAMetalLayer*)impl->webGPUView.layer;

#if MLN_WEBGPU_IMPL_DAWN
    if (!impl->instance) return;

    WGPUSurfaceSourceMetalLayer metalDesc = WGPU_SURFACE_SOURCE_METAL_LAYER_INIT;
    metalDesc.layer = (__bridge void*)metalLayer;

    WGPUSurfaceDescriptor surfaceDesc = WGPU_SURFACE_DESCRIPTOR_INIT;
    surfaceDesc.nextInChain = &metalDesc.chain;

    WGPUInstance rawInstance = impl->instance->Get();
    impl->surface = wgpuInstanceCreateSurface(rawInstance, &surfaceDesc);
#elif MLN_WEBGPU_IMPL_WGPU
    if (!impl->instance) return;

    WGPUSurfaceSourceMetalLayer metalDesc = {};
    metalDesc.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
    metalDesc.chain.next = nullptr;
    metalDesc.layer = (__bridge void*)metalLayer;

    WGPUSurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = &metalDesc.chain;

    impl->surface = wgpuInstanceCreateSurface(static_cast<WGPUInstance>(impl->instance), &surfaceDesc);
#endif

    if (!impl->surface) {
        mbgl::Log::Error(mbgl::Event::Render, "WebGPU iOS: Failed to create surface");
    }
}

void MLNMapViewWebGPUImpl::configureSurface(uint32_t width, uint32_t height) {
    if (!impl->surface || !impl->device || width == 0 || height == 0) return;

    WGPUSurfaceCapabilities capabilities = {};
#if MLN_WEBGPU_IMPL_DAWN
    WGPUStatus capStatus = wgpuSurfaceGetCapabilities(impl->surface, impl->adapter, &capabilities);
#elif MLN_WEBGPU_IMPL_WGPU
    WGPUStatus capStatus = wgpuSurfaceGetCapabilities(impl->surface, static_cast<WGPUAdapter>(impl->adapter), &capabilities);
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

void MLNMapViewWebGPUImpl::createDepthStencilTexture(uint32_t width, uint32_t height) {
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
        mbgl::Log::Warning(mbgl::Event::Render, "WebGPU iOS: Failed to create depth/stencil texture");
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
        mbgl::Log::Warning(mbgl::Event::Render, "WebGPU iOS: Failed to create depth/stencil view");
    }
}

void* MLNMapViewWebGPUImpl::getCurrentTextureView() {
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
    if (status == WGPUSurfaceGetCurrentTextureStatus_Outdated ||
        status == WGPUSurfaceGetCurrentTextureStatus_Lost) {
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

void MLNMapViewWebGPUImpl::presentSurface() {
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

void* MLNMapViewWebGPUImpl::getDepthStencilView() {
    return impl->depthStencilView;
}

mbgl::Size MLNMapViewWebGPUImpl::getFramebufferSize() const {
    return impl->framebufferSize;
}

UIView* MLNMapViewWebGPUImpl::getView() {
    return impl->webGPUView;
}

void MLNMapViewWebGPUImpl::setOpaque(const bool opaque) {
    impl->webGPUView.opaque = opaque;
    impl->webGPUView.layer.opaque = opaque;
}

void MLNMapViewWebGPUImpl::setPresentsWithTransaction(const bool value) {
    impl->presentsWithTransaction = value;

    if (@available(iOS 13.0, *)) {
        if (CAMetalLayer* metalLayer = MLN_OBJC_DYNAMIC_CAST(impl->webGPUView.layer, CAMetalLayer)) {
            metalLayer.presentsWithTransaction = value;
        }
    }
}

void MLNMapViewWebGPUImpl::display() {
    render();
}

void MLNMapViewWebGPUImpl::deleteView() {
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
            impl->surfaceConfigured = false;
        }
        wgpuSurfaceRelease(impl->surface);
        impl->surface = nullptr;
    }

    [impl->webGPUView removeFromSuperview];
    impl->webGPUView = nil;
}

void MLNMapViewWebGPUImpl::layoutChanged() {
    const auto scaleFactor = MLNEffectiveScaleFactorForView(mapView);
    const auto screenSize = mapView.bounds.size;

    impl->webGPUView.contentScaleFactor = scaleFactor;

    CGSize drawableSize = CGSizeMake(screenSize.width * scaleFactor,
                                     screenSize.height * scaleFactor);

    CAMetalLayer* metalLayer = (CAMetalLayer*)impl->webGPUView.layer;
    metalLayer.drawableSize = drawableSize;

    uint32_t w = static_cast<uint32_t>(drawableSize.width);
    uint32_t h = static_cast<uint32_t>(drawableSize.height);

    impl->framebufferSize = {w, h};
    size = impl->framebufferSize;

    if (impl->currentTextureView) {
        wgpuTextureViewRelease(impl->currentTextureView);
        impl->currentTextureView = nullptr;
    }
    if (impl->currentTexture) {
        wgpuTextureRelease(impl->currentTexture);
        impl->currentTexture = nullptr;
    }

    if (w > 0 && h > 0) {
        configureSurface(w, h);
    }
}

UIImage* MLNMapViewWebGPUImpl::snapshot() {
    return nil; // TODO: implement snapshot
}

MLNBackendResource* MLNMapViewWebGPUImpl::getObject() {
    return [[MLNBackendResource alloc] init];
}

void MLNMapViewWebGPUImpl::activate() {
    if (impl->activationCount++) {
        return;
    }
}

void MLNMapViewWebGPUImpl::deactivate() {
    if (--impl->activationCount) {
        return;
    }
}
