#pragma once

#include "glfw_backend.hpp"
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <memory>
#include <webgpu/webgpu_cpp.h>

struct GLFWwindow;
struct WGPUDeviceImpl;
struct WGPUSurfaceImpl;

namespace dawn::native {
class Instance;
class Adapter;
}

// Multiple inheritance: GLFWBackend for window management, 
// webgpu::RendererBackend for rendering, gfx::Renderable for framebuffer
class GLFWWebGPUBackend final : public GLFWBackend,
                                 public mbgl::webgpu::RendererBackend,
                                 public mbgl::gfx::Renderable {
public:
    GLFWWebGPUBackend(GLFWwindow* window, bool capFrameRate);
    ~GLFWWebGPUBackend() override;

    void swap();

    // GLFWBackend implementation
public:
    mbgl::gfx::RendererBackend& getRendererBackend() override;
    mbgl::Size getSize() const override;
    void setSize(mbgl::Size) override;

    // mbgl::gfx::RendererBackend implementation
public:
    mbgl::gfx::Renderable& getDefaultRenderable() override; 
    

protected:
    void activate() override;
    void deactivate() override;

    // WebGPU-specific methods
public:
    WGPUDeviceImpl* getWGPUDevice() { return device; }
    WGPUSurfaceImpl* getWGPUSurface() { return surface; }
    wgpu::TextureFormat getSwapChainFormat() const { return swapChainFormat; }
    
    // Override virtual methods from RendererBackend
    void* getCurrentTextureView() override;
    mbgl::Size getFramebufferSize() const override;

private:
    GLFWwindow* window;
    std::unique_ptr<dawn::native::Instance> instance;
    WGPUDeviceImpl* device = nullptr;
    WGPUSurfaceImpl* surface = nullptr;
    void* metalLayer = nullptr; // CAMetalLayer on macOS
    wgpu::Device wgpuDevice;
    wgpu::Queue queue;
    wgpu::Surface wgpuSurface;
    wgpu::TextureFormat swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
};