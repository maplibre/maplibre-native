#import "MLNMapView+Impl.h"
#import "MLNMapView_Private.h"

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gl/renderer_backend.hpp>

@class MLNMapViewImplDelegate;

/// Adapter responsible for bridging calls from mbgl to MLNMapView and Cocoa.
class MLNMapViewOpenGLImpl final : public MLNMapViewImpl,
                                   public mbgl::gl::RendererBackend,
                                   public mbgl::gfx::Renderable {
public:
    MLNMapViewOpenGLImpl(MLNMapView*);
    ~MLNMapViewOpenGLImpl() override;

public:
    void restoreFramebufferBinding();

#ifdef MLN_RECREATE_GL_IN_AN_EMERGENCY
private:
    void emergencyRecreateGL();
#endif

    // Implementation of mbgl::gfx::RendererBackend
public:
    mbgl::gfx::Renderable& getDefaultRenderable() override {
        return *this;
    }

private:
    void activate() override;
    void deactivate() override;
    // End implementation of mbgl::gfx::RendererBackend

    // Implementation of mbgl::gl::RendererBackend
public:
    void updateAssumedState() override;

private:
    mbgl::gl::ProcAddress getExtensionFunctionPointer(const char* name) override;
    // End implementation of mbgl::gl::Rendererbackend

    // Implementation of MLNMapViewImpl
public:
    mbgl::gfx::RendererBackend& getRendererBackend() override {
        return *this;
    }

    EAGLContext* getEAGLContext() override;
    void setOpaque(bool) override;
    void display() override;
    void setPresentsWithTransaction(bool) override;
    void createView() override;
    UIView* getView() override;
    void deleteView() override;
    UIImage* snapshot() override;
    void layoutChanged() override;
    // End implementation of MLNMapViewImpl
};
