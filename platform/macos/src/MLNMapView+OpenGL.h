#import "MLNMapView+Impl.h"
#import "MLNMapView_Private.h"

#include <mln/gfx/renderable.hpp>
#include <mln/gl/renderer_backend.hpp>

/// Adapter responsible for bridging calls from mbgl to MLNMapView and Cocoa.
class MLNMapViewOpenGLImpl final : public MLNMapViewImpl, public mln::gl::RendererBackend, public mln::gfx::Renderable {
public:
    MLNMapViewOpenGLImpl(MLNMapView*);
    ~MLNMapViewOpenGLImpl() override = default;

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

private:
    mln::gl::ProcAddress getExtensionFunctionPointer(const char* name) override;
    // End implementation of mln::gl::Rendererbackend

    // Implementation of MLNMapViewImpl
public:
    mln::gfx::RendererBackend& getRendererBackend() override { return *this; }

    mln::PremultipliedImage readStillImage() override;
    CGLContextObj getCGLContextObj() override;
};
