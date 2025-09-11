#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
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

    gfx::Renderable& getDefaultRenderable() override;
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