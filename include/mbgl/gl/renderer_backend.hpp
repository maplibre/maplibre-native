#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gl/resource_upload_thread_pool.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/size.hpp>
#include <mbgl/util/util.hpp>

namespace mbgl {

class ProgramParameters;

namespace gl {

using ProcAddress = void (*)();
using FramebufferID = uint32_t;

class UploadThreadContext {
public:
    UploadThreadContext() = default;
    virtual ~UploadThreadContext() = default;
    virtual void createContext() = 0;
    virtual void destroyContext() = 0;
    virtual void bindContext() = 0;
    virtual void unbindContext() = 0;
};

class RendererBackend : public gfx::RendererBackend {
public:
    RendererBackend(gfx::ContextMode);
    RendererBackend(gfx::ContextMode, const TaggedScheduler&);
    ~RendererBackend() override;

    /// Called prior to rendering to update the internally assumed OpenGL state.
    virtual void updateAssumedState() = 0;

#if MLN_DRAWABLE_RENDERER
    /// One-time shader initialization
    void initShaders(gfx::ShaderRegistry&, const ProgramParameters& programParameters) override;
#endif

    virtual bool supportFreeThreadedUpload() { return false; }
    virtual std::shared_ptr<UploadThreadContext> createUploadThreadContext() { return nullptr; }
    gl::ResourceUploadThreadPool& getResourceUploadThreadPool();

protected:
    std::unique_ptr<gfx::Context> createContext() override;

    /// Called with the name of an OpenGL extension that should be loaded.
    /// RendererBackend implementations must call the API-specific version that
    /// obtains the function pointer for this function, or a null pointer if
    /// unsupported/unavailable.
    virtual ProcAddress getExtensionFunctionPointer(const char*) = 0;

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

    void destroyResourceUploadThreadPool() { resourceUploadThreadPool = nullptr; }

public:
    /// Triggers an OpenGL state update if the internal assumed state doesn't
    /// match the supplied values.
    void setFramebufferBinding(FramebufferID fbo);
    void setViewport(int32_t x, int32_t y, const Size&);
    void setScissorTest(bool);

private:
    std::unique_ptr<gl::ResourceUploadThreadPool> resourceUploadThreadPool;
};

} // namespace gl
} // namespace mbgl
