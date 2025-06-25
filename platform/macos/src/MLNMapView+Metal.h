#import "MLNMapView+Impl.h"
#import "MLNMapView_Private.h"

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/mtl/renderer_backend.hpp>

@class MLNMapViewImplDelegate;

/// Adapter responsible for bridging calls from mbgl to MLNMapView and Cocoa.
class MLNMapViewMetalImpl final : public MLNMapViewImpl,
                                  public mbgl::mtl::RendererBackend,
                                  public mbgl::gfx::Renderable {
 public:
  MLNMapViewMetalImpl(MLNMapView*);
  ~MLNMapViewMetalImpl() override;

 public:
  void restoreFramebufferBinding();

  // Implementation of mbgl::gfx::RendererBackend
 public:
  mbgl::gfx::Renderable& getDefaultRenderable() override { return *this; }

 private:
  void activate() override;
  void deactivate() override;
  // End implementation of mbgl::gfx::RendererBackend

  // Implementation of mbgl::gl::RendererBackend
 public:
  void updateAssumedState() override;
  // End implementation of mbgl::gl::Rendererbackend

  // Implementation of MLNMapViewImpl
 public:
  mbgl::gfx::RendererBackend& getRendererBackend() override { return *this; }

  mbgl::PremultipliedImage readStillImage() override;

  MLNBackendResource* getObject() override;

 private:
  bool presentsWithTransaction = false;
};
