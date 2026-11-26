#import "MLNMapView+OpenGL.h"
#import "MLNOpenGLLayer.h"

#include <mln/gl/renderable_resource.hpp>

#import <OpenGL/gl.h>

class MLNMapViewOpenGLRenderableResource final : public mln::gl::RenderableResource {
public:
  MLNMapViewOpenGLRenderableResource(MLNMapViewOpenGLImpl& backend_) : backend(backend_) {}

  void bind() override { backend.restoreFramebufferBinding(); }

private:
  MLNMapViewOpenGLImpl& backend;

public:
  // The current framebuffer of the NSOpenGLLayer we are painting to.
  GLint fbo = 0;

  // The reference counted count of activation calls
  NSUInteger activationCount = 0;
};

MLNMapViewOpenGLImpl::MLNMapViewOpenGLImpl(MLNMapView* nativeView_)
    : MLNMapViewImpl(nativeView_),
      mln::gl::RendererBackend(mln::gfx::ContextMode::Unique),
      mln::gfx::Renderable(mapView.framebufferSize,
                           std::make_unique<MLNMapViewOpenGLRenderableResource>(*this)) {
  // Install the OpenGL layer. Interface Builder’s synchronous drawing means
  // we can’t display a map, so don’t even bother to have a map layer.
  mapView.layer = mapView.isTargetingInterfaceBuilder ? [CALayer layer] : [MLNOpenGLLayer layer];
}

mln::gl::ProcAddress MLNMapViewOpenGLImpl::getExtensionFunctionPointer(const char* name) {
  static CFBundleRef framework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
  if (!framework) {
    throw std::runtime_error("Failed to load OpenGL framework.");
  }

  return reinterpret_cast<mln::gl::ProcAddress>(CFBundleGetFunctionPointerForName(
      framework, (__bridge CFStringRef)[NSString stringWithUTF8String:name]));
}

void MLNMapViewOpenGLImpl::activate() {
  auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
  if (resource.activationCount++) {
    return;
  }

  MLNOpenGLLayer* layer = (MLNOpenGLLayer*)mapView.layer;
  [layer.openGLContext makeCurrentContext];
}

void MLNMapViewOpenGLImpl::deactivate() {
  auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
  if (--resource.activationCount) {
    return;
  }

  [NSOpenGLContext clearCurrentContext];
}

void MLNMapViewOpenGLImpl::updateAssumedState() {
  auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &resource.fbo);
  assumeFramebufferBinding(resource.fbo);
  assumeViewport(0, 0, mapView.framebufferSize);
}

void MLNMapViewOpenGLImpl::restoreFramebufferBinding() {
  auto& resource = getResource<MLNMapViewOpenGLRenderableResource>();
  setFramebufferBinding(resource.fbo);
  setViewport(0, 0, mapView.framebufferSize);
}

mln::PremultipliedImage MLNMapViewOpenGLImpl::readStillImage() {
  return readFramebuffer(mapView.framebufferSize);
}

CGLContextObj MLNMapViewOpenGLImpl::getCGLContextObj() {
  MLNOpenGLLayer* layer = (MLNOpenGLLayer*)mapView.layer;
  return layer.openGLContext.CGLContextObj;
}
