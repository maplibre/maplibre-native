#include <mbgl/webgpu/headless_backend.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/webgpu/wgpu_cpp_compat.hpp>

#if MLN_WEBGPU_IMPL_DAWN
#include <dawn/native/DawnNative.h>
#elif MLN_WEBGPU_IMPL_WGPU
#include <wgpu.h>
#endif

namespace mbgl {
namespace webgpu {

class HeadlessRenderableResource final : public webgpu::RenderableResource {
public:
    HeadlessRenderableResource(HeadlessBackend&, webgpu::Context& context_, Size size)
        : context(context_),
          size(size) {
        // Create offscreen texture using the proper WebGPU OffscreenTexture class
        offscreenTexture = context.createOffscreenTexture(size, gfx::TextureChannelDataType::UnsignedByte, true, true);
    }

    ~HeadlessRenderableResource() noexcept override = default;

    void bind() override {
        // Bind the offscreen texture
        if (offscreenTexture) {
            offscreenTexture->getResource<webgpu::RenderableResource>().bind();
        }
    }

    void swap() override {
        // For headless rendering, we don't need to swap
    }

    const mbgl::webgpu::RendererBackend& getBackend() const override { return context.getBackend(); }

    const WGPUCommandEncoder& getCommandEncoder() const override {
        // This will be set by the command encoder when needed
        static WGPUCommandEncoder dummy = nullptr;
        return dummy;
    }

    WGPURenderPassEncoder getRenderPassEncoder() const override {
        // This will be set by the render pass when needed
        return nullptr;
    }

    WGPUTextureView getColorTextureView() override {
        if (offscreenTexture) {
            return offscreenTexture->getResource<webgpu::RenderableResource>().getColorTextureView();
        }
        return nullptr;
    }

    std::optional<wgpu::TextureFormat> getColorTextureFormat() const override {
        if (offscreenTexture) {
            return offscreenTexture->getResource<webgpu::RenderableResource>().getColorTextureFormat();
        }
        return std::nullopt;
    }

    WGPUTextureView getDepthStencilTextureView() override {
        if (offscreenTexture) {
            return offscreenTexture->getResource<webgpu::RenderableResource>().getDepthStencilTextureView();
        }
        return nullptr;
    }

    std::optional<wgpu::TextureFormat> getDepthStencilTextureFormat() const override {
        if (offscreenTexture) {
            return offscreenTexture->getResource<webgpu::RenderableResource>().getDepthStencilTextureFormat();
        }
        return std::nullopt;
    }

    PremultipliedImage readStillImage() {
        if (offscreenTexture) {
            return offscreenTexture->readStillImage();
        }
        return PremultipliedImage(size);
    }

private:
    webgpu::Context& context;
    Size size;
    std::unique_ptr<gfx::OffscreenTexture> offscreenTexture;
};

class HeadlessBackend::Impl {
public:
#if MLN_WEBGPU_IMPL_DAWN
    std::unique_ptr<dawn::native::Instance> instance;
#elif MLN_WEBGPU_IMPL_WGPU
    wgpu::Instance instance;
    wgpu::Adapter adapter;
#endif
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

#if MLN_WEBGPU_IMPL_DAWN
    // Initialize Dawn instance with TimedWaitAny feature enabled
    wgpu::InstanceFeatureName timedWaitAnyFeature = wgpu::InstanceFeatureName::TimedWaitAny;
    wgpu::InstanceDescriptor instanceDesc = {};
    instanceDesc.requiredFeatureCount = 1;
    instanceDesc.requiredFeatures = &timedWaitAnyFeature;

    impl->instance = std::make_unique<dawn::native::Instance>(&instanceDesc);

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
#elif MLN_WEBGPU_IMPL_WGPU
    // wgpu-native backend initialization
    wgpu::InstanceDescriptor instanceDesc = {};
    impl->instance = wgpu::createInstance(instanceDesc);

    if (!impl->instance) {
        Log::Error(Event::Render, "WebGPU HeadlessBackend: Failed to create instance");
        return;
    }

    // Request adapter using simpler blocking API
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.powerPreference = wgpu::PowerPreference::HighPerformance;

    impl->adapter = impl->instance.requestAdapter(adapterOpts);

    if (!impl->adapter) {
        Log::Error(Event::Render, "WebGPU HeadlessBackend: No adapter found");
        return;
    }

    // Request device using simpler blocking API
    wgpu::DeviceDescriptor deviceDesc = {};

    impl->device = impl->adapter.requestDevice(deviceDesc);

    if (!impl->device) {
        Log::Error(Event::Render, "WebGPU HeadlessBackend: Failed to create device");
        return;
    }

    impl->queue = impl->device.getQueue();

    // Store device, queue, and instance in base class
    setDevice(static_cast<WGPUDevice>(impl->device));
    setQueue(static_cast<WGPUQueue>(impl->queue));
    setInstance(static_cast<WGPUInstance>(impl->instance));
#endif

    // Create offscreen render texture
    createOffscreenTextures();
}

HeadlessBackend::~HeadlessBackend() {
    // Explicitly reset the renderable resource
    setResource(nullptr);

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

#if MLN_WEBGPU_IMPL_DAWN
    colorDesc.dimension = wgpu::TextureDimension::e2D;
#elif MLN_WEBGPU_IMPL_WGPU
    colorDesc.dimension = wgpu::TextureDimension::_2D;
#endif
    colorDesc.size = {
        static_cast<uint32_t>(impl->framebufferSize.width), static_cast<uint32_t>(impl->framebufferSize.height), 1};
    colorDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    colorDesc.mipLevelCount = 1;
    colorDesc.sampleCount = 1;

#if MLN_WEBGPU_IMPL_DAWN
    colorDesc.label = "Headless Color Texture";
    impl->offscreenTexture = impl->device.CreateTexture(&colorDesc);
#elif MLN_WEBGPU_IMPL_WGPU
    impl->offscreenTexture = impl->device.createTexture(colorDesc);

#endif

    if (impl->offscreenTexture) {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;

#if MLN_WEBGPU_IMPL_DAWN
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
#elif MLN_WEBGPU_IMPL_WGPU
        viewDesc.dimension = wgpu::TextureViewDimension::_2D;
#endif
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = wgpu::TextureAspect::All;
#if MLN_WEBGPU_IMPL_DAWN
        viewDesc.label = "Headless Color TextureView";

        impl->offscreenTextureView = impl->offscreenTexture.CreateView(&viewDesc);
#elif MLN_WEBGPU_IMPL_WGPU
        impl->offscreenTextureView = impl->offscreenTexture.createView(viewDesc);
#endif
    }

    // Create depth/stencil texture
    wgpu::TextureDescriptor depthDesc = {};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment;

#if MLN_WEBGPU_IMPL_DAWN
    depthDesc.dimension = wgpu::TextureDimension::e2D;
#elif MLN_WEBGPU_IMPL_WGPU
    depthDesc.dimension = wgpu::TextureDimension::_2D;
#endif
    depthDesc.size = {
        static_cast<uint32_t>(impl->framebufferSize.width), static_cast<uint32_t>(impl->framebufferSize.height), 1};
    depthDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount = 1;

#if MLN_WEBGPU_IMPL_DAWN
    depthDesc.label = "Headless Depth/Stencil Texture";

    impl->depthTexture = impl->device.CreateTexture(&depthDesc);
#elif MLN_WEBGPU_IMPL_WGPU
    impl->depthTexture = impl->device.createTexture(depthDesc);
#endif

    if (impl->depthTexture) {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;

#if MLN_WEBGPU_IMPL_DAWN
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
#elif MLN_WEBGPU_IMPL_WGPU
        viewDesc.dimension = wgpu::TextureViewDimension::_2D;
#endif
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = wgpu::TextureAspect::All;

#if MLN_WEBGPU_IMPL_DAWN
        viewDesc.label = "Headless Depth/Stencil TextureView";

        impl->depthTextureView = impl->depthTexture.CreateView(&viewDesc);
#elif MLN_WEBGPU_IMPL_WGPU
        impl->depthTextureView = impl->depthTexture.createView(viewDesc);
#endif
    }
}

gfx::Renderable& HeadlessBackend::getDefaultRenderable() {
    if (!hasResource()) {
        setResource(
            std::make_unique<HeadlessRenderableResource>(*this, static_cast<webgpu::Context&>(getContext()), size));
    }
    return *this;
}

PremultipliedImage HeadlessBackend::readStillImage() {
    if (hasResource()) {
        return getResource<HeadlessRenderableResource>().readStillImage();
    }
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
    if (hasResource()) {
        return getResource<HeadlessRenderableResource>().getColorTextureView();
    }
    return nullptr;
}

void* HeadlessBackend::getDepthStencilView() {
    if (hasResource()) {
        return getResource<HeadlessRenderableResource>().getDepthStencilTextureView();
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
    Size size, gfx::Renderable::SwapBehaviour swapBehaviour, gfx::ContextMode contextMode) {
    return std::make_unique<webgpu::HeadlessBackend>(size, swapBehaviour, contextMode);
}

} // namespace gfx
} // namespace mbgl
