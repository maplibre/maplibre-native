#import "MLNMapView+Impl.h"
#import "MLNMapView_Private.h"

#include <mln/gfx/renderable.hpp>
#include <mln/mtl/renderer_backend.hpp>

@class MLNMapViewImplDelegate;

/// Adapter responsible for bridging calls from mbgl to MLNMapView and Cocoa.
class MLNMapViewMetalImpl final : public MLNMapViewImpl,
                                  public mln::mtl::RendererBackend,
                                  public mln::gfx::Renderable {
public:
  MLNMapViewMetalImpl(MLNMapView*);
  ~MLNMapViewMetalImpl() override;

public:
  void restoreFramebufferBinding();

  // Implementation of mln::gfx::RendererBackend
public:
  mln::gfx::Renderable& getDefaultRenderable() override { return *this; }

private:
  void activate() override;
  void deactivate() override;
  // End implementation of mln::gfx::RendererBackend

  // Implementation of mln::gl::RendererBackend
public:
  void updateAssumedState() override;
  // End implementation of mln::gl::Rendererbackend

  // Implementation of MLNMapViewImpl
public:
  mln::gfx::RendererBackend& getRendererBackend() override { return *this; }

  mln::PremultipliedImage readStillImage() override;
  MLNBackendResource* getObject() override;
  void display() override;

  void drawableSizeChanged(CGSize size);

private:
  bool presentsWithTransaction = true;
};
