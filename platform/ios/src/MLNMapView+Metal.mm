#import "MLNDisplayUtils.h"
#import "MLNFoundation_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNMapView+Metal.h"

#import <mbgl/mtl/renderable_resource.hpp>

#import <MetalKit/MetalKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#import <Metal/Metal.hpp>

@interface MLNMapViewImplDelegate : NSObject <MTKViewDelegate>
@end

@implementation MLNMapViewImplDelegate {
    MLNMapViewMetalImpl* _impl;
}

- (instancetype)initWithImpl:(MLNMapViewMetalImpl*)impl {
    if (self = [super init]) {
        _impl = impl;
    }
    return self;
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {}

- (void)drawInMTKView:(MTKView *)view {
    _impl->render();
}

@end

class MLNMapViewMetalRenderableResource final : public mbgl::mtl::RenderableResource {
public:
    MLNMapViewMetalRenderableResource(MLNMapViewMetalImpl& backend_)
        : backend(backend_),
          delegate([[MLNMapViewImplDelegate alloc] initWithImpl:&backend]) {
    }

    void bind() override {
        if (!commandQueue) {
            commandQueue = [mtlView.device newCommandQueue];
        }

        if (!commandBuffer) {
            commandBuffer = [commandQueue commandBuffer];
            commandBufferPtr = NS::RetainPtr((__bridge MTL::CommandBuffer*)commandBuffer);
        }
    }

    const mbgl::mtl::RendererBackend& getBackend() const override { return backend; }

    const mbgl::mtl::MTLCommandBufferPtr& getCommandBuffer() const override {
        return commandBufferPtr;
    }

    virtual mbgl::mtl::MTLBlitPassDescriptorPtr getUploadPassDescriptor() const override {
        // Create from render pass descriptor?
        return NS::TransferPtr(MTL::BlitPassDescriptor::alloc()->init());
    }

    const mbgl::mtl::MTLRenderPassDescriptorPtr& getRenderPassDescriptor() const override {
        if (!cachedRenderPassDescriptor) {
            auto* mtlDesc = mtlView.currentRenderPassDescriptor;
            cachedRenderPassDescriptor = NS::RetainPtr((__bridge MTL::RenderPassDescriptor*)mtlDesc);
        }
        return cachedRenderPassDescriptor;
    }

    void swap() override {
        id<CAMetalDrawable> currentDrawable = [mtlView currentDrawable];
        if (currentDrawable) {
            if (presentsWithTransaction) {
                [commandBuffer commit];
                [commandBuffer waitUntilCompleted];
                [currentDrawable present];
            } else {
                [commandBuffer presentDrawable:currentDrawable];
                [commandBuffer commit];
            }
        }

        commandBuffer = nil;
        commandBufferPtr.reset();

        cachedRenderPassDescriptor.reset();
    }

    mbgl::Size framebufferSize() {
        assert(mtlView);
        return { static_cast<uint32_t>(mtlView.drawableSize.width),
                 static_cast<uint32_t>(mtlView.drawableSize.height) };
    }

private:
    MLNMapViewMetalImpl& backend;
    mbgl::mtl::MTLCommandBufferPtr commandBufferPtr;
    mutable mbgl::mtl::MTLRenderPassDescriptorPtr cachedRenderPassDescriptor;

public:
    MLNMapViewImplDelegate* delegate = nil;
    MTKView *mtlView = nil;
    id <MTLCommandBuffer> commandBuffer;
    id <MTLCommandQueue> commandQueue;
    bool presentsWithTransaction = false;

    // We count how often the context was activated/deactivated so that we can truly deactivate it
    // after the activation count drops to 0.
    NSUInteger activationCount = 0;
};

MLNMapViewMetalImpl::MLNMapViewMetalImpl(MLNMapView* nativeView_)
    : MLNMapViewImpl(nativeView_),
      mbgl::mtl::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::gfx::Renderable({ 0, 0 }, std::make_unique<MLNMapViewMetalRenderableResource>(*this)) {
}

MLNMapViewMetalImpl::~MLNMapViewMetalImpl() = default;

void MLNMapViewMetalImpl::setOpaque(const bool opaque) {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    resource.mtlView.opaque = opaque;
    resource.mtlView.layer.opaque = opaque;
}

void MLNMapViewMetalImpl::setPresentsWithTransaction(const bool value) {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    resource.presentsWithTransaction = value;

    if (@available(iOS 13.0, *)) {
        if (CAMetalLayer* metalLayer = MLN_OBJC_DYNAMIC_CAST(resource.mtlView.layer, CAMetalLayer)) {
            metalLayer.presentsWithTransaction = value;
        }
    }
}

void MLNMapViewMetalImpl::display() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();

    // Calling `display` here directly causes the stuttering bug (if
    // `presentsWithTransaction` is `YES` - see above)
    // as reported in https://github.com/mapbox/mapbox-gl-native-ios/issues/350
    //
    // Since we use `presentsWithTransaction` to synchronize with UIView
    // annotations, we now let the system handle when the view is rendered. This
    // has the potential to increase latency
    [resource.mtlView setNeedsDisplay];
}

void MLNMapViewMetalImpl::createView() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    if (resource.mtlView) {
        return;
    }

    id<MTLDevice> device = (__bridge id<MTLDevice>)resource.getBackend().getDevice().get();
    const auto scaleFactor = MLNEffectiveScaleFactorForView(mapView);

    resource.mtlView = [[MTKView alloc] initWithFrame:mapView.bounds device:device];
    resource.mtlView.delegate = resource.delegate;
    resource.mtlView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    resource.mtlView.contentScaleFactor = scaleFactor;
    resource.mtlView.contentMode = UIViewContentModeCenter;
    resource.mtlView.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    resource.mtlView.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    resource.mtlView.opaque = mapView.opaque;
    resource.mtlView.layer.opaque = mapView.opaque;
    resource.mtlView.enableSetNeedsDisplay = YES;
    if (@available(iOS 13.0, *)) {
        CAMetalLayer* metalLayer = MLN_OBJC_DYNAMIC_CAST(resource.mtlView.layer, CAMetalLayer);
        metalLayer.presentsWithTransaction = resource.presentsWithTransaction;
    }

    [mapView insertSubview:resource.mtlView atIndex:0];
}

UIView* MLNMapViewMetalImpl::getView() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    return resource.mtlView;
}

void MLNMapViewMetalImpl::deleteView() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    [resource.mtlView releaseDrawables];
}

void MLNMapViewMetalImpl::activate() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    if (resource.activationCount++) {
        return;
    }
}

void MLNMapViewMetalImpl::deactivate() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    if (--resource.activationCount) {
        return;
    }
}

/// This function is called before we start rendering, when iOS invokes our rendering method.
/// iOS already sets the correct framebuffer and viewport for us, so we need to update the
/// context state with the anticipated values.
void MLNMapViewMetalImpl::updateAssumedState() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    assumeFramebufferBinding(ImplicitFramebufferBinding);
    assumeViewport(0, 0, resource.framebufferSize());
}

UIImage* MLNMapViewMetalImpl::snapshot() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    return nil; //TODO: resource.mtlView.snapshot;
}

void MLNMapViewMetalImpl::layoutChanged() {
    const auto scaleFactor = MLNEffectiveScaleFactorForView(mapView);
    const auto screenSize = mapView.bounds.size;

    auto& resource = getResource<MLNMapViewMetalRenderableResource>();

    resource.mtlView.contentScaleFactor = scaleFactor;
    resource.mtlView.drawableSize = CGSizeMake(screenSize.width * scaleFactor, screenSize.height * scaleFactor);

    size = { static_cast<uint32_t>(resource.mtlView.drawableSize.width),
             static_cast<uint32_t>(resource.mtlView.drawableSize.height) };
}

MLNBackendResource* MLNMapViewMetalImpl::getObject() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    auto renderPassDescriptor = resource.getRenderPassDescriptor().get();

    return [[MLNBackendResource alloc] initWithMTKView:resource.mtlView
                                                device:resource.mtlView.device
                                  renderPassDescriptor:[MTLRenderPassDescriptor renderPassDescriptor]
                                         commandBuffer:resource.commandBuffer];
}
