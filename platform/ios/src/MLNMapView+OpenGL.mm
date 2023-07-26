#import "MLNFoundation_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNMapView+OpenGL.h"

#include <mbgl/gl/renderable_resource.hpp>

#import <GLKit/GLKit.h>
#import <OpenGLES/EAGL.h>
#import <QuartzCore/CAEAGLLayer.h>

@interface MLNMapViewImplDelegate : NSObject <GLKViewDelegate>
@end

@implementation MLNMapViewImplDelegate {
    MLNMapViewOpenGLImpl* _impl;
}

- (instancetype)initWithImpl:(MLNMapViewOpenGLImpl*)impl {
    if (self = [super init]) {
        _impl = impl;
    }
    return self;
}

- (void)glkView:(nonnull GLKView*)view drawInRect:(CGRect)rect {
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

class MLNMapViewOpenGLRenderableResource final : public mbgl::gl::RenderableResource {
public:
    MLNMapViewOpenGLRenderableResource(MLNMapViewOpenGLImpl& backend_)
        : backend(backend_),
          delegate([[MLNMapViewImplDelegate alloc] initWithImpl:&backend]),
          atLeastiOS_12_2_0([NSProcessInfo.processInfo
              isOperatingSystemAtLeastVersion:(NSOperatingSystemVersion){ 12, 2, 0 }]) {
    }

    void bind() override {
        backend.restoreFramebufferBinding();
    }

    mbgl::Size framebufferSize() {
        assert(glView);
        return { static_cast<uint32_t>(glView.drawableWidth),
                 static_cast<uint32_t>(glView.drawableHeight) };
    }

private:
    MLNMapViewOpenGLImpl& backend;

public:
    MLNMapViewImplDelegate* delegate = nil;
    GLKView *glView = nil;
    EAGLContext *context = nil;
    const bool atLeastiOS_12_2_0;

    // We count how often the context was activated/deactivated so that we can truly deactivate it
    // after the activation count drops to 0.
    NSUInteger activationCount = 0;
};

MLNMapViewOpenGLImpl::MLNMapViewOpenGLImpl(MLNMapView* nativeView_)
    : MLNMapViewImpl(nativeView_),
      mbgl::gl::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::gfx::Renderable({ 0, 0 }, std::make_unique<MLNMapViewOpenGLRenderableResource>(*this)) {
}

MLNMapViewOpenGLImpl::~MLNMapViewOpenGLImpl() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    if (resource.context && [[EAGLContext currentContext] isEqual:resource.context]) {
        [EAGLContext setCurrentContext:nil];
    }
}

void MLNMapViewOpenGLImpl::setOpaque(const bool opaque) {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    resource.glView.opaque = opaque;
    resource.glView.layer.opaque = opaque;
}

void MLNMapViewOpenGLImpl::setPresentsWithTransaction(const bool value) {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    CAEAGLLayer* eaglLayer = MLN_OBJC_DYNAMIC_CAST(resource.glView.layer, CAEAGLLayer);
    eaglLayer.presentsWithTransaction = value;
}

void MLNMapViewOpenGLImpl::display() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();

    // Calling `display` here directly causes the stuttering bug (if
    // `presentsWithTransaction` is `YES` - see above)
    // as reported in https://github.com/mapbox/mapbox-gl-native-ios/issues/350
    //
    // Since we use `presentsWithTransaction` to synchronize with UIView
    // annotations, we now let the system handle when the view is rendered. This
    // has the potential to increase latency
    [resource.glView setNeedsDisplay];
}

void MLNMapViewOpenGLImpl::createView() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    if (resource.glView) {
        return;
    }

    if (!resource.context) {
        resource.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        assert(resource.context);
    }

    resource.glView = [[GLKView alloc] initWithFrame:mapView.bounds context:resource.context];
    resource.glView.delegate = resource.delegate;
    resource.glView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    resource.glView.contentScaleFactor = contentScaleFactor();
    resource.glView.contentMode = UIViewContentModeCenter;
    resource.glView.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    resource.glView.drawableDepthFormat = GLKViewDrawableDepthFormat16;
    resource.glView.opaque = mapView.opaque;
    resource.glView.layer.opaque = mapView.opaque;
    resource.glView.enableSetNeedsDisplay = YES;
    CAEAGLLayer* eaglLayer = MLN_OBJC_DYNAMIC_CAST(resource.glView.layer, CAEAGLLayer);
    eaglLayer.presentsWithTransaction = NO;

    [mapView insertSubview:resource.glView atIndex:0];
}

UIView* MLNMapViewOpenGLImpl::getView() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    return resource.glView;
}

void MLNMapViewOpenGLImpl::deleteView() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    [resource.glView deleteDrawable];
}

#ifdef MLN_RECREATE_GL_IN_AN_EMERGENCY
// TODO: Fix or remove
// See https://github.com/mapbox/mapbox-gl-native/issues/14232
void MLNMapViewOpenGLImpl::emergencyRecreateGL() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    MLNLogError(@"Rendering took too long - creating GL views");

    CAEAGLLayer* eaglLayer = MLN_OBJC_DYNAMIC_CAST(resource.glView.layer, CAEAGLLayer);
    eaglLayer.presentsWithTransaction = NO;

    [mapView pauseRendering:nil];

    // Just performing a pauseRendering:/resumeRendering: pair isn't sufficient - in this case
    // we can still get errors when calling bindDrawable. Here we completely
    // recreate the GLKView

    [mapView.userLocationAnnotationView removeFromSuperview];
    [resource.glView removeFromSuperview];

    // Recreate the view
    resource.glView = nil;
    createView();

    if (mapView.annotationContainerView) {
        [resource.glView insertSubview:mapView.annotationContainerView atIndex:0];
    }

    [mapView updateUserLocationAnnotationView];

    // Do not bind...yet

    if (mapView.window) {
        [mapView resumeRendering:nil];
        eaglLayer = MLN_OBJC_DYNAMIC_CAST(resource.glView.layer, CAEAGLLayer);
        eaglLayer.presentsWithTransaction = mapView.enablePresentsWithTransaction;
    } else {
        MLNLogDebug(@"No window - skipping resumeRendering");
    }
}
#endif

mbgl::gl::ProcAddress MLNMapViewOpenGLImpl::getExtensionFunctionPointer(const char* name) {
    static CFBundleRef framework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengles"));
    if (!framework) {
        throw std::runtime_error("Failed to load OpenGL framework.");
    }

    return reinterpret_cast<mbgl::gl::ProcAddress>(CFBundleGetFunctionPointerForName(
        framework, (__bridge CFStringRef)[NSString stringWithUTF8String:name]));
}

void MLNMapViewOpenGLImpl::activate() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    if (resource.activationCount++) {
        return;
    }

    [EAGLContext setCurrentContext:resource.context];
}

void MLNMapViewOpenGLImpl::deactivate() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    if (--resource.activationCount) {
        return;
    }

    [EAGLContext setCurrentContext:nil];
}

/// This function is called before we start rendering, when iOS invokes our rendering method.
/// iOS already sets the correct framebuffer and viewport for us, so we need to update the
/// context state with the anticipated values.
void MLNMapViewOpenGLImpl::updateAssumedState() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    assumeFramebufferBinding(ImplicitFramebufferBinding);
    assumeViewport(0, 0, resource.framebufferSize());
}

void MLNMapViewOpenGLImpl::restoreFramebufferBinding() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    if (!implicitFramebufferBound()) {
        // Something modified our state, and we need to bind the original drawable again.
        // Doing this also sets the viewport to the full framebuffer.
        // Note that in reality, iOS does not use the Framebuffer 0 (it's typically 1), and we
        // only use this is a placeholder value.
        [resource.glView bindDrawable];
        updateAssumedState();
    } else {
        // Our framebuffer is still bound, but the viewport might have changed.
        setViewport(0, 0, resource.framebufferSize());
    }
}

UIImage* MLNMapViewOpenGLImpl::snapshot() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    return resource.glView.snapshot;
}

void MLNMapViewOpenGLImpl::layoutChanged() {
    const auto scaleFactor = contentScaleFactor();
    size = { static_cast<uint32_t>(mapView.bounds.size.width * scaleFactor),
             static_cast<uint32_t>(mapView.bounds.size.height * scaleFactor) };
}

EAGLContext* MLNMapViewOpenGLImpl::getEAGLContext() {
    auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
    return resource.context;
}
