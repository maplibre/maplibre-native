#import "MLNMapView+Impl.h"
#import "MLNMapView_Private.h"

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>

/// Adapter responsible for bridging calls from mbgl to MLNMapView and Cocoa (WebGPU backend).
class MLNMapViewWebGPUImpl final : public MLNMapViewImpl,
                                    public mbgl::webgpu::RendererBackend,
                                    public mbgl::gfx::Renderable {
public:
    MLNMapViewWebGPUImpl(MLNMapView*);
    ~MLNMapViewWebGPUImpl() override;

    // Implementation of mbgl::gfx::RendererBackend
public:
    mbgl::gfx::Renderable& getDefaultRenderable() override { return *this; }

    // webgpu::RendererBackend overrides
    void* getCurrentTextureView() override;
    void* getDepthStencilView() override;
    mbgl::Size getFramebufferSize() const override;
    void presentSurface();

    // Implementation of MLNMapViewImpl
public:
    mbgl::gfx::RendererBackend& getRendererBackend() override { return *this; }

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
