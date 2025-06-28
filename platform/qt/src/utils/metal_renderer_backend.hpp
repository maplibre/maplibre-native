#pragma once

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_OSX || TARGET_OS_IPHONE

#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/context.hpp>
#import <QuartzCore/CAMetalLayer.hpp>
#include <QtCore/QtGlobal>

namespace QMapLibre {

class MetalRendererBackend final : public mbgl::mtl::RendererBackend, public mbgl::gfx::Renderable {
public:
    explicit MetalRendererBackend(CA::MetalLayer* layer);
    // Fallback ctor used by MapRenderer when only a ContextMode is provided.
    explicit MetalRendererBackend(mbgl::gfx::ContextMode /*mode*/);
    ~MetalRendererBackend() override;

    // mbgl::gfx::RendererBackend ------------------------------------------------
    mbgl::gfx::Renderable& getDefaultRenderable() override { return static_cast<mbgl::gfx::Renderable&>(*this); }
    void activate() override {}
    void deactivate() override {}
    void updateAssumedState() override {}

    // Qt-specific --------------------------------------------------------------
    void setSize(mbgl::Size size_);
    mbgl::Size getSize() const;

    // Returns the color texture of the drawable rendered in the last frame.
    void* currentDrawable() const { return m_currentDrawable; }

    void _q_setCurrentDrawable(void* tex) { m_currentDrawable = tex; }

    // Qt Widgets path still expects this hook even though Metal does not use an
    // OpenGL FBO.  Provide a no-op so code that is agnostic of the backend can
    // compile unmodified.
    void updateFramebuffer(quint32 /*fbo*/, const mbgl::Size& /*size*/) {}

private:
    void* m_currentDrawable{nullptr}; // id<MTLTexture>
    MetalRendererBackend(const MetalRendererBackend&) = delete;
    MetalRendererBackend& operator=(const MetalRendererBackend&) = delete;

    friend class QtMetalRenderableResource;
};

} // namespace QMapLibre

#endif // TARGET_OS_OSX || TARGET_OS_IPHONE
#endif // __APPLE__
