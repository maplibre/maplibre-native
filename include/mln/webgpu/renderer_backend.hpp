#pragma once

#include <mln/gfx/renderer_backend.hpp>
#include <mln/util/size.hpp>

#if MLN_WEBGPU_IMPL_DAWN
#include <webgpu/webgpu.h>
#elif MLN_WEBGPU_IMPL_WGPU
#include <webgpu.h>
#endif

#include <mln/webgpu/wgpu_cpp_compat.hpp>
#include <memory>

namespace mln {

class ProgramParameters;

namespace gfx {
class ShaderRegistry;
}

namespace webgpu {

class Context;

class RendererBackend : public gfx::RendererBackend {
public:
    explicit RendererBackend(gfx::ContextMode);
    ~RendererBackend() override;

    // getDefaultRenderable() is pure virtual - must be implemented by platform backends
    // Platform backends typically inherit from gfx::Renderable and return *this
    void initShaders(gfx::ShaderRegistry&, const ProgramParameters&) override;

    // Platform-specific surface creation
    void setSurface(void* nativeWindow);

    // WebGPU-specific methods
    // Emdawn hosts that use synchronous readback must create the instance with
    // WGPUInstanceFeatureName_TimedWaitAny enabled.
    void setInstance(void* instance);
    void setDevice(WGPUDevice device);
    void setQueue(WGPUQueue queue);
    void* getInstance() const;
    WGPUDevice getDevice() const;
    WGPUQueue getQueue() const;
    void* getSurface() const;

    // Surface texture access - can be overridden by platform backends
    virtual void* getCurrentTextureView();
    virtual void* getDepthStencilView();
    virtual mln::Size getFramebufferSize() const;

    void setDepthStencilFormat(wgpu::TextureFormat);
    wgpu::TextureFormat getDepthStencilFormat() const;
    void setColorFormat(wgpu::TextureFormat);
    wgpu::TextureFormat getColorFormat() const;

protected:
    std::unique_ptr<gfx::Context> createContext() override;
    void activate() override;
    void deactivate() override;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace webgpu
} // namespace mln
