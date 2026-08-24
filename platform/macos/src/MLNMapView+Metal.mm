#import "MLNFoundation_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNMapView+Metal.h"

#import <mln/mtl/renderable_resource.hpp>

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
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

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size {
  _impl->drawableSizeChanged(size);
}

- (void)drawInMTKView:(MTKView*)view {
  _impl->render();
}

@end

class MLNMapViewMetalRenderableResource final : public mln::mtl::RenderableResource {
public:
  MLNMapViewMetalRenderableResource(MLNMapViewMetalImpl& backend_)
      : backend(backend_), delegate([[MLNMapViewImplDelegate alloc] initWithImpl:&backend]) {}

  void bind() override {
    if (!commandQueue) {
      commandQueue = [mtlView.device newCommandQueue];
    }

    if (!commandBuffer) {
      commandBuffer = [commandQueue commandBuffer];
      commandBufferPtr = NS::RetainPtr((__bridge MTL::CommandBuffer*)commandBuffer);
    }
  }

  const mln::mtl::RendererBackend& getBackend() const override { return backend; }

  const mln::mtl::MTLCommandBufferPtr& getCommandBuffer() const override {
    return commandBufferPtr;
  }

  virtual mln::mtl::MTLBlitPassDescriptorPtr getUploadPassDescriptor() const override {
    // Create from render pass descriptor?
    return NS::TransferPtr(MTL::BlitPassDescriptor::alloc()->init());
  }

  const mln::mtl::MTLRenderPassDescriptorPtr& getRenderPassDescriptor() const override {
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

  mln::Size framebufferSize() {
    assert(mtlView);
    return {static_cast<uint32_t>(mtlView.drawableSize.width),
            static_cast<uint32_t>(mtlView.drawableSize.height)};
  }

private:
  MLNMapViewMetalImpl& backend;
  mln::mtl::MTLCommandBufferPtr commandBufferPtr;
  mutable mln::mtl::MTLRenderPassDescriptorPtr cachedRenderPassDescriptor;

public:
  MLNMapViewImplDelegate* delegate = nil;
  MTKView* mtlView = nil;
  id<MTLCommandBuffer> commandBuffer;
  id<MTLCommandQueue> commandQueue;
  bool presentsWithTransaction = false;

  // We count how often the context was activated/deactivated so that we can truly deactivate it
  // after the activation count drops to 0.
  NSUInteger activationCount = 0;
};

MLNMapViewMetalImpl::MLNMapViewMetalImpl(MLNMapView* nativeView_)
    : MLNMapViewImpl(nativeView_),
      mln::mtl::RendererBackend(mln::gfx::ContextMode::Unique),
      mln::gfx::Renderable({0, 0}, std::make_unique<MLNMapViewMetalRenderableResource>(*this)) {
  auto& resource = getResource<MLNMapViewMetalRenderableResource>();
  if (resource.mtlView) {
    return;
  }

  id<MTLDevice> device = (__bridge id<MTLDevice>)resource.getBackend().getDevice().get();

  resource.mtlView = [[MTKView alloc] initWithFrame:mapView.bounds device:device];
  resource.mtlView.delegate = resource.delegate;
  resource.mtlView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  resource.mtlView.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
  resource.mtlView.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
  resource.mtlView.layer.opaque = mapView.opaque;
  resource.mtlView.paused = YES;
  resource.mtlView.enableSetNeedsDisplay = YES;
  CAMetalLayer* metalLayer = MLN_OBJC_DYNAMIC_CAST(resource.mtlView.layer, CAMetalLayer);
  metalLayer.presentsWithTransaction = presentsWithTransaction;
  resource.presentsWithTransaction = presentsWithTransaction;

  [mapView addSubview:resource.mtlView positioned:NSWindowBelow relativeTo:nil];
  drawableSizeChanged(resource.mtlView.drawableSize);
}

void MLNMapViewMetalImpl::drawableSizeChanged(CGSize drawableSize) {
  size = {static_cast<uint32_t>(drawableSize.width), static_cast<uint32_t>(drawableSize.height)};
}

MLNMapViewMetalImpl::~MLNMapViewMetalImpl() = default;

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

mln::PremultipliedImage MLNMapViewMetalImpl::readStillImage() {
  // return readFramebuffer(mapView.framebufferSize); // TODO: RendererBackend::readFramebuffer
  return {};
}

void MLNMapViewMetalImpl::display() {
  auto& resource = getResource<MLNMapViewMetalRenderableResource>();
  resource.mtlView.needsDisplay = YES;
}

MLNBackendResource* MLNMapViewMetalImpl::getObject() {
  auto& resource = getResource<MLNMapViewMetalRenderableResource>();

  return [[MLNBackendResource alloc] initWithMTKView:resource.mtlView
                                              device:resource.mtlView.device
                                renderPassDescriptor:resource.mtlView.currentRenderPassDescriptor
                                       commandBuffer:resource.commandBuffer];
}
