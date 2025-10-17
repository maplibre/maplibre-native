#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/util/size.hpp>

#if MLN_WEBGPU_IMPL_DAWN
#include <webgpu/webgpu.h>
#elif MLN_WEBGPU_IMPL_WGPU
#include <webgpu.h>
#endif

#include <mbgl/webgpu/wgpu_cpp_compat.hpp>
#include <memory>

namespace mbgl {

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
    void setInstance(void* instance);
    void setDevice(void* device);
    void setQueue(void* queue);
    void* getInstance() const;
    void* getDevice() const;
    void* getQueue() const;
    void* getSurface() const;

    // Surface texture access - can be overridden by platform backends
    virtual void* getCurrentTextureView();
    virtual void* getDepthStencilView();
    virtual mbgl::Size getFramebufferSize() const;

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
} // namespace mbgl
