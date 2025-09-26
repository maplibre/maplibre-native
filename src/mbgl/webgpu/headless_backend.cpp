#include <mbgl/webgpu/headless_backend.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/webgpu/wgpu_cpp_compat.hpp>
#include <dawn/native/DawnNative.h>

namespace mbgl {
namespace webgpu {

class HeadlessBackend::Impl {
public:
    std::unique_ptr<dawn::native::Instance> instance;
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::Texture offscreenTexture;
    wgpu::TextureView offscreenTextureView;
    wgpu::Texture depthTexture;
    wgpu::TextureView depthTextureView;
    Size framebufferSize;
};

HeadlessBackend::HeadlessBackend(Size size_, SwapBehaviour swapBehaviour_, gfx::ContextMode mode)
    : webgpu::RendererBackend(mode),
      gfx::HeadlessBackend(size_),
      impl(std::make_unique<Impl>()) {

    static_cast<void>(swapBehaviour_);

    impl->framebufferSize = size_;

    // Initialize Dawn instance
    impl->instance = std::make_unique<dawn::native::Instance>();

    // Enumerate adapters using the correct API
    std::vector<dawn::native::Adapter> adapters = impl->instance->EnumerateAdapters();

    if (adapters.empty()) {
        Log::Error(Event::Render, "WebGPU HeadlessBackend: No adapters found");
        return;
    }

    // Use the first adapter (typically Vulkan on Linux)
    dawn::native::Adapter& selectedAdapter = adapters[0];

    // Create device descriptor
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = "MapLibre Headless WebGPU Device";

    // Create device
    WGPUDevice rawDevice = selectedAdapter.CreateDevice(&deviceDesc);
    if (!rawDevice) {
        Log::Error(Event::Render, "WebGPU HeadlessBackend: Failed to create device");
        return;
    }

    impl->device = wgpu::Device::Acquire(rawDevice);
    impl->queue = impl->device.GetQueue();

    // Store device and queue in base class
    setDevice(impl->device.Get());
    setQueue(impl->queue.Get());
    setInstance(impl->instance->Get());

    // Create offscreen render texture
    createOffscreenTextures();
}

HeadlessBackend::~HeadlessBackend() {
    if (impl) {
        // Clean up textures
        impl->offscreenTextureView = nullptr;
        impl->offscreenTexture = nullptr;
        impl->depthTextureView = nullptr;
        impl->depthTexture = nullptr;

        // Clean up device and queue
        impl->queue = nullptr;
        impl->device = nullptr;
        impl->instance = nullptr;
    }
}

void HeadlessBackend::createOffscreenTextures() {
    if (!impl->device) return;

    // Release old textures if they exist
    impl->offscreenTextureView = nullptr;
    impl->offscreenTexture = nullptr;
    impl->depthTextureView = nullptr;
    impl->depthTexture = nullptr;

    // Create offscreen color texture
    wgpu::TextureDescriptor colorDesc = {};
    colorDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    colorDesc.dimension = wgpu::TextureDimension::e2D;
    colorDesc.size = {static_cast<uint32_t>(impl->framebufferSize.width),
                     static_cast<uint32_t>(impl->framebufferSize.height), 1};
    colorDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    colorDesc.mipLevelCount = 1;
    colorDesc.sampleCount = 1;
    colorDesc.label = "Headless Color Texture";

    impl->offscreenTexture = impl->device.CreateTexture(&colorDesc);

    if (impl->offscreenTexture) {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = wgpu::TextureAspect::All;
        viewDesc.label = "Headless Color TextureView";

        impl->offscreenTextureView = impl->offscreenTexture.CreateView(&viewDesc);
    }

    // Create depth/stencil texture
    wgpu::TextureDescriptor depthDesc = {};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment;
    depthDesc.dimension = wgpu::TextureDimension::e2D;
    depthDesc.size = {static_cast<uint32_t>(impl->framebufferSize.width),
                      static_cast<uint32_t>(impl->framebufferSize.height), 1};
    depthDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount = 1;
    depthDesc.label = "Headless Depth/Stencil Texture";

    impl->depthTexture = impl->device.CreateTexture(&depthDesc);

    if (impl->depthTexture) {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = wgpu::TextureAspect::All;
        viewDesc.label = "Headless Depth/Stencil TextureView";

        impl->depthTextureView = impl->depthTexture.CreateView(&viewDesc);
    }
}

gfx::Renderable& HeadlessBackend::getDefaultRenderable() {
    return *this;
}

PremultipliedImage HeadlessBackend::readStillImage() {
    // TODO: Implement texture readback with Dawn API
    // The Dawn API for buffer mapping and texture-to-buffer copy has changed significantly.
    // For now, return an empty image.
    return PremultipliedImage(getSize());
}

RendererBackend* HeadlessBackend::getRendererBackend() {
    return this;
}

void HeadlessBackend::activate() {
    active = true;
    // Ensure textures are created if size changed
    if (impl && impl->framebufferSize != getSize()) {
        impl->framebufferSize = getSize();
        createOffscreenTextures();
    }
}

void HeadlessBackend::deactivate() {
    active = false;
}

void* HeadlessBackend::getCurrentTextureView() {
    if (impl && impl->offscreenTextureView) {
        return impl->offscreenTextureView.Get();
    }
    return nullptr;
}

void* HeadlessBackend::getDepthStencilView() {
    if (impl && impl->depthTextureView) {
        return impl->depthTextureView.Get();
    }
    return nullptr;
}

mbgl::Size HeadlessBackend::getFramebufferSize() const {
    if (impl) {
        return impl->framebufferSize;
    }
    return getSize();
}

} // namespace webgpu

namespace gfx {

template <>
std::unique_ptr<gfx::HeadlessBackend> Backend::Create<Backend::Type::WebGPU>(
    Size size,
    gfx::Renderable::SwapBehaviour swapBehaviour,
    gfx::ContextMode contextMode) {
    return std::make_unique<webgpu::HeadlessBackend>(size, swapBehaviour, contextMode);
}

} // namespace gfx
} // namespace mbgl
