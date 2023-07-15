#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/size.hpp>
#include <mbgl/util/util.hpp>

namespace mbgl {

class ProgramParameters;

namespace mtl {

using ProcAddress = void (*)();
using FramebufferID = uint32_t;

class RendererBackend : public gfx::RendererBackend {
public:
    RendererBackend(gfx::ContextMode);
    ~RendererBackend() override;

    /// Called prior to rendering to update the internally assumed OpenGL state.
    virtual void updateAssumedState() = 0;

#if MLN_DRAWABLE_RENDERER
    /// One-time shader initialization
    void initShaders(gfx::ShaderRegistry&, const ProgramParameters& programParameters) override;
#endif

protected:
    std::unique_ptr<gfx::Context> createContext() override;

    /// Reads the color pixel data from the currently bound framebuffer.
    PremultipliedImage readFramebuffer(const Size&);

    /// A constant to signal that a framebuffer is bound, but with an unknown ID.
    static constexpr const FramebufferID ImplicitFramebufferBinding = std::numeric_limits<FramebufferID>::max();

    /// Tells the renderer that OpenGL state has already been set by the windowing toolkit.
    /// It sets the internal assumed state to the supplied values.
    void assumeFramebufferBinding(FramebufferID fbo);
    void assumeViewport(int32_t x, int32_t y, const Size&);
    void assumeScissorTest(bool);

    /// Returns true when assumed framebuffer binding hasn't changed from the implicit binding.
    bool implicitFramebufferBound();

public:
    /// Triggers an OpenGL state update if the internal assumed state doesn't
    /// match the supplied values.
    void setFramebufferBinding(FramebufferID fbo);
    void setViewport(int32_t x, int32_t y, const Size&);
    void setScissorTest(bool);
};

} // namespace mtl
} // namespace mbgl
