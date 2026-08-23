#import "MLNMapView+Impl.h"
#import "MLNMapView_Private.h"

#include <mln/gfx/renderable.hpp>
#include <mln/webgpu/renderer_backend.hpp>

/// Adapter responsible for bridging calls from mbgl to MLNMapView and Cocoa (WebGPU backend).
class MLNMapViewWebGPUImpl final : public MLNMapViewImpl,
                                   public mln::webgpu::RendererBackend,
                                   public mln::gfx::Renderable {
public:
  MLNMapViewWebGPUImpl(MLNMapView*);
  ~MLNMapViewWebGPUImpl() override;

  // Implementation of mln::gfx::RendererBackend
public:
  mln::gfx::Renderable& getDefaultRenderable() override { return *this; }

  // webgpu::RendererBackend overrides
  void* getCurrentTextureView() override;
  void* getDepthStencilView() override;
  mln::Size getFramebufferSize() const override;
  void presentSurface();
  void markNeedsPresent();

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

private:
  void activate() override;
  void deactivate() override;

  void createSurface();
  void configureSurface(uint32_t width, uint32_t height);
  void createDepthStencilTexture(uint32_t width, uint32_t height);

  class Impl;
  std::unique_ptr<Impl> impl;
};
