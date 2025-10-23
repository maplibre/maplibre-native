#pragma once

#include "glfw_backend.hpp"
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <memory>
#include <webgpu/webgpu_cpp.h>
#include <queue>
#include <atomic>
#include <array>

struct GLFWwindow;
struct WGPUDeviceImpl;
struct WGPUSurfaceImpl;

#if MLN_WEBGPU_IMPL_DAWN
namespace dawn::native {
class Instance;
class Adapter;
} // namespace dawn::native
#endif

// Multiple inheritance: GLFWBackend for window management,
// webgpu::RendererBackend for rendering, gfx::Renderable for framebuffer
class GLFWWebGPUBackend final : public GLFWBackend, public mbgl::webgpu::RendererBackend, public mbgl::gfx::Renderable {
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
    // Override virtual methods from RendererBackend
    void* getCurrentTextureView() override;
    void* getDepthStencilView() override;
    mbgl::Size getFramebufferSize() const override;

private:
    class SpinLock {
    public:
        void lock();
        void unlock();

    private:
        std::atomic_flag flag = ATOMIC_FLAG_INIT;
    };

    mutable SpinLock textureStateLock;

    GLFWwindow* window;

#if MLN_WEBGPU_IMPL_DAWN
    std::unique_ptr<dawn::native::Instance> instance;
#elif MLN_WEBGPU_IMPL_WGPU
    wgpu::Instance wgpuInstance;
#endif
    wgpu::Device wgpuDevice; // This owns the device
    wgpu::Queue queue;
    wgpu::Surface wgpuSurface;
    wgpu::TextureFormat swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::TextureFormat depthStencilFormat = wgpu::TextureFormat::Undefined;
    std::array<wgpu::TextureFormat, 1> configuredViewFormats{wgpu::TextureFormat::BGRA8Unorm};

    // Protect texture view access from multiple threads
    wgpu::TextureView currentTextureView; // Keep the current texture view alive
    wgpu::Texture currentTexture;         // Keep the texture alive as well

    // Depth/stencil texture and view
    wgpu::Texture depthStencilTexture;
    wgpu::TextureView depthStencilView;

    // Frame synchronization
    std::atomic<bool> frameInProgress{false};
    // Track if we have presented the current frame
    std::atomic<bool> framePresented{true};

    // Track if we're shutting down to prevent mutex usage during destruction
    std::atomic<bool> isShuttingDown{false};

    // Previous frame resources - keep alive until next swap
    wgpu::TextureView previousTextureView;
    wgpu::Texture previousTexture;

    // Surface state tracking
    std::atomic<bool> surfaceConfigured{false};
    std::atomic<bool> surfaceNeedsReconfigure{false};
    mbgl::Size lastConfiguredSize{0, 0};

    // Error recovery
    std::atomic<int> consecutiveErrors{0};
    static constexpr int maxConsecutiveErrors = 3;

    // VSync control
    bool enableVSync = true;

    // Helper methods
    void reconfigureSurface();
    void createDepthStencilTexture(uint32_t width, uint32_t height);
    bool waitForFrame(std::chrono::milliseconds timeout = std::chrono::milliseconds(100));
    void signalFrameComplete();
    void periodicMaintenance();
    void processEvents();
};
