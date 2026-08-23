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

  void setOpaque(bool) override;
  void display() override;
  void setPresentsWithTransaction(bool) override;
  void createView() override;
  UIView* getView() override;
  void deleteView() override;
  UIImage* snapshot() override;
  void layoutChanged() override;
  MLNBackendResource* getObject() override;
  // End implementation of MLNMapViewImpl
};
