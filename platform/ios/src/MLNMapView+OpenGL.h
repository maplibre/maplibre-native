#import "MLNMapView+Impl.h"
#import "MLNMapView_Private.h"

#include <mln/gfx/renderable.hpp>
#include <mln/gl/renderer_backend.hpp>

@class MLNMapViewImplDelegate;

/// Adapter responsible for bridging calls from mbgl to MLNMapView and Cocoa.
class MLNMapViewOpenGLImpl final : public MLNMapViewImpl,
                                   public mln::gl::RendererBackend,
                                   public mln::gfx::Renderable {
public:
  MLNMapViewOpenGLImpl(MLNMapView*);
  ~MLNMapViewOpenGLImpl() override;

public:
  void restoreFramebufferBinding();

#ifdef MLN_RECREATE_GL_IN_AN_EMERGENCY
private:
  void emergencyRecreateGL();
#endif

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

private:
  mln::gl::ProcAddress getExtensionFunctionPointer(const char* name) override;
  // End implementation of mln::gl::Rendererbackend

  // Implementation of MLNMapViewImpl
public:
  mln::gfx::RendererBackend& getRendererBackend() override { return *this; }

  EAGLContext* getEAGLContext() override;
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
