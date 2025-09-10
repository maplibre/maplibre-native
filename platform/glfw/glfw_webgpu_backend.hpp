#pragma once

#include "glfw_backend.hpp"
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <memory>

struct GLFWwindow;
struct WGPUDeviceImpl;
struct WGPUSurfaceImpl;

namespace dawn::native {
class Instance;
class Adapter;
}

class GLFWWebGPUBackend final : public GLFWBackend, 
                                public mbgl::webgpu::RendererBackend, 
                                public mbgl::gfx::Renderable {
public:
    GLFWWebGPUBackend(GLFWwindow* window, bool capFrameRate);
    ~GLFWWebGPUBackend() override;

    void swap();

    // GLFWBackend implementation
public:
    mbgl::gfx::RendererBackend& getRendererBackend() override { return *this; }
    mbgl::Size getSize() const override;
    void setSize(mbgl::Size) override;

    // mbgl::gfx::RendererBackend implementation
public:
    mbgl::gfx::Renderable& getDefaultRenderable() override { return *this; }

protected:
    void activate() override;
    void deactivate() override;

    // mbgl::webgpu::RendererBackend implementation
protected:
    void updateAssumedState() override;
    WGPUDeviceImpl* getDevice() override { return device; }
    WGPUSurfaceImpl* getSurface() override { return surface; }

private:
    GLFWwindow* window;
    std::unique_ptr<dawn::native::Instance> instance;
    WGPUDeviceImpl* device = nullptr;
    WGPUSurfaceImpl* surface = nullptr;
    void* metalLayer = nullptr; // CAMetalLayer on macOS
};