#import "MLNFoundation_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNMapView+Metal.h"

#include <mbgl/mtl/renderable_resource.hpp>

#import <MetalKit/MetalKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

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

namespace {
CGFloat contentScaleFactor() {
    return [UIScreen instancesRespondToSelector:@selector(nativeScale)]
        ? [[UIScreen mainScreen] nativeScale]
        : [[UIScreen mainScreen] scale];
}
} // namespace

class MLNMapViewMetalRenderableResource final : public mbgl::mtl::RenderableResource {
public:
    MLNMapViewMetalRenderableResource(MLNMapViewMetalImpl& backend_)
        : backend(backend_),
          delegate([[MLNMapViewImplDelegate alloc] initWithImpl:&backend]),
          atLeastiOS_12_2_0([NSProcessInfo.processInfo
              isOperatingSystemAtLeastVersion:(NSOperatingSystemVersion){ 12, 2, 0 }]) {
    }

    void bind() override {
        if (!commandQueue) {
            commandQueue = [mtlView.device newCommandQueue];
        }
        commandBuffer = [commandQueue commandBuffer];
        currentDrawable = [mtlView currentDrawable];
    }
    
    void swap() override {
        [commandBuffer presentDrawable:currentDrawable];
        [commandBuffer commit];
    }

    mbgl::Size framebufferSize() {
        assert(mtlView);
        return { static_cast<uint32_t>(mtlView.drawableSize.width),
                 static_cast<uint32_t>(mtlView.drawableSize.height) };
    }

private:
    MLNMapViewMetalImpl& backend;

public:
    MLNMapViewImplDelegate* delegate = nil;
    MTKView *mtlView = nil;
    id<CAMetalDrawable> currentDrawable;
    id <MTLCommandBuffer> commandBuffer;
    id <MTLCommandQueue> commandQueue;
    const bool atLeastiOS_12_2_0;

    // We count how often the context was activated/deactivated so that we can truly deactivate it
    // after the activation count drops to 0.
    NSUInteger activationCount = 0;
};

MLNMapViewMetalImpl::MLNMapViewMetalImpl(MLNMapView* nativeView_)
    : MLNMapViewImpl(nativeView_),
      mbgl::mtl::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::gfx::Renderable({ 0, 0 }, std::make_unique<MLNMapViewMetalRenderableResource>(*this)) {
}

MLNMapViewMetalImpl::~MLNMapViewMetalImpl() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    /*if (resource.context && [[EAGLContext currentContext] isEqual:resource.context]) {
        [EAGLContext setCurrentContext:nil];
    }*/
}

void MLNMapViewMetalImpl::setOpaque(const bool opaque) {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    resource.mtlView.opaque = opaque;
    resource.mtlView.layer.opaque = opaque;
}

void MLNMapViewMetalImpl::setPresentsWithTransaction(const bool value) {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    CAMetalLayer* metalLayer = MLN_OBJC_DYNAMIC_CAST(resource.mtlView.layer, CAMetalLayer);
    metalLayer.presentsWithTransaction = value;
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

    /*if (!resource.context) {
        resource.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        assert(resource.context);
    }*/

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    
    resource.mtlView = [[MTKView alloc] initWithFrame:mapView.bounds device:device];
    resource.mtlView.delegate = resource.delegate;
    resource.mtlView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    resource.mtlView.contentScaleFactor = contentScaleFactor();
    resource.mtlView.contentMode = UIViewContentModeCenter;
    resource.mtlView.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    resource.mtlView.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    resource.mtlView.opaque = mapView.opaque;
    resource.mtlView.layer.opaque = mapView.opaque;
    resource.mtlView.enableSetNeedsDisplay = YES;
    CAMetalLayer* metalLayer = MLN_OBJC_DYNAMIC_CAST(resource.mtlView.layer, CAMetalLayer);
    metalLayer.presentsWithTransaction = NO;

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

    //[EAGLContext setCurrentContext:resource.context];
}

void MLNMapViewMetalImpl::deactivate() {
    auto& resource = getResource<MLNMapViewMetalRenderableResource>();
    if (--resource.activationCount) {
        return;
    }

    //[EAGLContext setCurrentContext:nil];
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
    const auto scaleFactor = contentScaleFactor();
    size = { static_cast<uint32_t>(mapView.bounds.size.width * scaleFactor),
             static_cast<uint32_t>(mapView.bounds.size.height * scaleFactor) };
}
