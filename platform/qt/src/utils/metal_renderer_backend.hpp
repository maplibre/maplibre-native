// Copyright (C) 2024 MapLibre contributors
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_OSX

#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/context.hpp>
#import <QuartzCore/CAMetalLayer.hpp>
#include <QtCore/QtGlobal>

namespace QMapLibre {

/*! \brief Metal renderer backend for the Qt platform (macOS desktop).
 *
 *  This class adapts MapLibre Native's generic Metal backend to the Metal
 *  objects supplied by Qt Quick on macOS.  It mirrors the GLFW variant in
 *  `platform/glfw/metal_backend.{h,mm}` but receives a \c CAMetalLayer from
 *  Qt instead of a raw NSWindow.
 *
 *  The implementation currently focuses on getting the code to compile so that
 *  Qt Quick + Metal can be enabled during the build.  Full swap-chain handling
 *  will be filled in step-by-step (similar to the existing GLFW backend).
 */
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

#endif // TARGET_OS_OSX
#endif // __APPLE__
