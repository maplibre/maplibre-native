#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include <mbgl/util/logging.hpp>
#include <cstring>
#include <functional>

#if MLN_WEBGPU_IMPL_WGPU
#include <webgpu/wgpu.h>
#endif

namespace {
constexpr uint32_t bytesPerRowAlignment = 256u;
}

namespace mbgl {
namespace webgpu {

class OffscreenTextureResource final : public webgpu::RenderableResource {
public:
    OffscreenTextureResource(
        Context& context_, const Size size_, const gfx::TextureChannelDataType type_, bool depth, bool stencil)
        : context(context_),
          size(size_),
          type(type_) {
        assert(!size.isEmpty());

        // Create color texture
        colorTexture = context.createTexture2D();
        colorTexture->setSize(size);
        colorTexture->setFormat(gfx::TexturePixelType::RGBA, type);
        if (auto* webgpuTexture = static_cast<Texture2D*>(colorTexture.get())) {
            webgpuTexture->setUsage(WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst |
                                    WGPUTextureUsage_CopySrc | WGPUTextureUsage_RenderAttachment);
            webgpuTexture->setSizeChangedCallback([this](const Size& newSize) { handleResize(newSize); });
        }
        colorTexture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});

        // Create depth/stencil texture if needed
        // WebGPU requires a combined depth-stencil texture when both are needed
        if (depth || stencil) {
            depthTexture = context.createTexture2D();
            depthTexture->setSize(size);

            // Use combined depth-stencil format when both are needed
            if (depth && stencil) {
                depthTexture->setFormat(gfx::TexturePixelType::Stencil, gfx::TextureChannelDataType::UnsignedByte);
            } else if (depth) {
                depthTexture->setFormat(gfx::TexturePixelType::Depth, gfx::TextureChannelDataType::Float);
            } else {
                depthTexture->setFormat(gfx::TexturePixelType::Stencil, gfx::TextureChannelDataType::UnsignedByte);
            }

            if (auto* webgpuDepth = static_cast<Texture2D*>(depthTexture.get())) {
                webgpuDepth->setUsage(WGPUTextureUsage_RenderAttachment);
            }
            depthTexture->setSamplerConfiguration(
                {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        }
    }

    ~OffscreenTextureResource() noexcept override { context.renderingStats().numFrameBuffers--; }

    void bind() override {
        // Create textures if not already created
        colorTexture->create();

        if (depthTexture) {
            depthTexture->create();
        }
    }

    void swap() override {
        // Offscreen targets do not present to a surface.
    }

    const mbgl::webgpu::RendererBackend& getBackend() const override {
        return static_cast<const mbgl::webgpu::RendererBackend&>(context.getBackend());
    }

    const WGPUCommandEncoder& getCommandEncoder() const override {
        static WGPUCommandEncoder dummyEncoder = nullptr;
        return dummyEncoder;
    }

    WGPURenderPassEncoder getRenderPassEncoder() const override { return nullptr; }

    WGPUTextureView getColorTextureView() override {
        if (!colorTexture) {
            return nullptr;
        }

        auto& texture = static_cast<Texture2D&>(*colorTexture);
        texture.create();
        return texture.getTextureView();
    }

    std::optional<wgpu::TextureFormat> getColorTextureFormat() const override {
        if (!colorTexture) {
            return std::nullopt;
        }
        const auto* texture = static_cast<const Texture2D*>(colorTexture.get());
        return static_cast<wgpu::TextureFormat>(texture->getNativeFormat());
    }

    WGPUTextureView getDepthStencilTextureView() override {
        if (depthTexture) {
            auto& texture = static_cast<Texture2D&>(*depthTexture);
            texture.create();
            return texture.getTextureView();
        }
        return nullptr;
    }

    std::optional<wgpu::TextureFormat> getDepthStencilTextureFormat() const override {
        if (depthTexture) {
            const auto* texture = static_cast<const Texture2D*>(depthTexture.get());
            return static_cast<wgpu::TextureFormat>(texture->getNativeFormat());
        }
        return std::nullopt;
    }

    PremultipliedImage readStillImage() {
        // Read back the color texture data
        constexpr uint32_t bytesPerPixel = 4; // RGBA8 output
        const uint32_t rowStride = static_cast<uint32_t>(size.width) * bytesPerPixel;
        const uint32_t alignedRowStride = ((rowStride + bytesPerRowAlignment - 1) / bytesPerRowAlignment) *
                                          bytesPerRowAlignment;
        const uint64_t alignedDataSize = static_cast<uint64_t>(alignedRowStride) * size.height;

        auto data = std::make_unique<uint8_t[]>(static_cast<size_t>(rowStride) * size.height);

        // WebGPU texture readback requires:
        // 1. Ensure all rendering commands are submitted and completed
        // 2. Create a buffer with MAP_READ usage
        // 3. Copy texture to buffer using command encoder
        // 4. Map the buffer and read the data
        // This is an async operation in WebGPU, but we need sync behavior here

        auto& backend = static_cast<const RendererBackend&>(context.getBackend());
        auto device = static_cast<WGPUDevice>(backend.getDevice());
        auto queue = static_cast<WGPUQueue>(backend.getQueue());
        if (!device || !queue) {
            mbgl::Log::Error(mbgl::Event::Render, "WebGPU: No device or queue available for readback");
            return {size, std::move(data)};
        }

        // Process all pending device operations to ensure rendering is complete
#if MLN_WEBGPU_IMPL_DAWN
        wgpuDeviceTick(device);
#endif

        // Create staging buffer for readback
        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView bufferLabel = {"Readback Buffer", strlen("Readback Buffer")};
        bufferDesc.label = bufferLabel;
        bufferDesc.size = alignedDataSize;
        bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;

        WGPUBuffer stagingBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        if (!stagingBuffer) {
            mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Failed to create staging buffer");
            return {size, std::move(data)};
        }

        // Copy texture to buffer
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
        if (encoder) {
            auto& webgpuTexture = static_cast<Texture2D&>(*colorTexture);
            WGPUTexture texture = webgpuTexture.getTexture();

            if (texture) {
                WGPUTexelCopyTextureInfo src = {};
                src.texture = texture;
                src.mipLevel = 0;
                src.origin = {0, 0, 0};
                src.aspect = WGPUTextureAspect_All;

                WGPUTexelCopyBufferInfo dst = {};
                dst.buffer = stagingBuffer;
                dst.layout.offset = 0;
                dst.layout.bytesPerRow = alignedRowStride;
                dst.layout.rowsPerImage = size.height;

                WGPUExtent3D copySize = {static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height), 1};

                wgpuCommandEncoderCopyTextureToBuffer(encoder, &src, &dst, &copySize);

                WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
                wgpuQueueSubmit(queue, 1, &commands);
                wgpuCommandBufferRelease(commands);
            } else {
                mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Color texture is null");
            }
            wgpuCommandEncoderRelease(encoder);
        } else {
            mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Failed to create command encoder");
        }

        // Map buffer and read data (blocking)
        // Use WaitAny for proper synchronous blocking instead of polling
        struct MapContext {
            WGPUMapAsyncStatus status = WGPUMapAsyncStatus_Error;
            bool completed = false;
        };
        MapContext mapContext;

#if MLN_WEBGPU_IMPL_DAWN
        // Dawn: Use WaitAny for proper synchronous blocking
        WGPUBufferMapCallbackInfo callbackInfo = {};
        callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
        callbackInfo.callback =
            [](WGPUMapAsyncStatus status, WGPUStringView /*message*/, void* userdata1, void* /*userdata2*/) {
                auto* ctx = static_cast<MapContext*>(userdata1);
                ctx->status = status;
                ctx->completed = true;
            };
        callbackInfo.userdata1 = &mapContext;
        callbackInfo.userdata2 = nullptr;

        WGPUFuture future = wgpuBufferMapAsync(stagingBuffer, WGPUMapMode_Read, 0, alignedDataSize, callbackInfo);

        // Use WaitAny to block until mapping completes
        auto instance = static_cast<WGPUInstance>(backend.getInstance());
        if (instance) {
            WGPUFutureWaitInfo waitInfo = {};
            waitInfo.future = future;
            waitInfo.completed = false;

            // Wait indefinitely for the mapping to complete
            WGPUWaitStatus waitStatus = wgpuInstanceWaitAny(instance, 1, &waitInfo, UINT64_MAX);

            if (waitStatus != WGPUWaitStatus_Success || !waitInfo.completed) {
                mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Buffer mapping wait failed");
            }
        }
#elif MLN_WEBGPU_IMPL_WGPU

        // wgpu-native: Use polling approach (WaitAny not implemented yet)
        // Use AllowProcessEvents callback mode for synchronous polling
        WGPUBufferMapCallbackInfo callbackInfo = {};
        callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
        callbackInfo.callback =
            [](WGPUMapAsyncStatus status, WGPUStringView /*message*/, void* userdata1, void* /*userdata2*/) {
                auto* ctx = static_cast<MapContext*>(userdata1);
                ctx->status = status;
                ctx->completed = true;
            };
        callbackInfo.userdata1 = &mapContext;
        callbackInfo.userdata2 = nullptr;

        wgpuBufferMapAsync(stagingBuffer, WGPUMapMode_Read, 0, alignedDataSize, callbackInfo);

        // Poll device until mapping completes
        // wgpu-native processes callbacks during wgpuDevicePoll
        constexpr int maxIterations = 1000;
        for (int i = 0; i < maxIterations && !mapContext.completed; ++i) {
            wgpuDevicePoll(device, true, nullptr);
            if (!mapContext.completed) {
                // Small sleep to avoid busy-waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        if (!mapContext.completed) {
            mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Buffer mapping timeout after polling");
        }
#endif

        // Check if mapping was successful and read the data
        if (mapContext.status == WGPUMapAsyncStatus_Success) {
            const auto* mappedData = static_cast<const uint8_t*>(
                wgpuBufferGetConstMappedRange(stagingBuffer, 0, alignedDataSize));
            if (mappedData) {
                if (alignedRowStride == rowStride) {
                    std::memcpy(data.get(), mappedData, static_cast<size_t>(alignedDataSize));
                } else {
                    auto* dstPtr = data.get();
                    for (uint32_t row = 0; row < size.height; ++row) {
                        std::memcpy(dstPtr + static_cast<size_t>(row) * rowStride,
                                    mappedData + static_cast<size_t>(row) * alignedRowStride,
                                    rowStride);
                    }
                }
            } else {
                mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Failed to get mapped range");
            }
        } else {
            mbgl::Log::Error(mbgl::Event::Render,
                             "WebGPU: Buffer mapping failed with status: " + std::to_string(mapContext.status));
        }

        wgpuBufferUnmap(stagingBuffer);
        wgpuBufferRelease(stagingBuffer);

        return {size, std::move(data)};
    }

    gfx::Texture2DPtr& getTexture() {
        assert(colorTexture);
        return colorTexture;
    }

    void setSizeListener(std::function<void(const Size&)> listener) { sizeListener = std::move(listener); }

private:
    void handleResize(const Size& newSize) {
        if (newSize == size || newSize.isEmpty()) {
            return;
        }
        size = newSize;

        if (depthTexture) {
            static_cast<Texture2D&>(*depthTexture).setSize(newSize);
            static_cast<Texture2D&>(*depthTexture).create();
        }

        if (auto* color = static_cast<Texture2D*>(colorTexture.get())) {
            color->create();
        }

        if (sizeListener) {
            sizeListener(size);
        }
    }

    Context& context;
    Size size;
    const gfx::TextureChannelDataType type;
    gfx::Texture2DPtr colorTexture;
    gfx::Texture2DPtr depthTexture;
    std::function<void(const Size&)> sizeListener;
};

OffscreenTexture::OffscreenTexture(
    Context& context, const Size size_, const gfx::TextureChannelDataType type, bool depth, bool stencil)
    : gfx::OffscreenTexture(size_, std::make_unique<OffscreenTextureResource>(context, size_, type, depth, stencil)) {
    auto& offscreenResource = getResource<OffscreenTextureResource>();
    offscreenResource.setSizeListener([this](const Size& newSize) { size = newSize; });
}

bool OffscreenTexture::isRenderable() {
    // Check if the texture resource is valid and can be rendered to
    try {
        auto& res = getResource<OffscreenTextureResource>();
        return res.getTexture() != nullptr;
    } catch (...) {
        return false;
    }
}

PremultipliedImage OffscreenTexture::readStillImage() {
    return getResource<OffscreenTextureResource>().readStillImage();
}

const gfx::Texture2DPtr& OffscreenTexture::getTexture() {
    return getResource<OffscreenTextureResource>().getTexture();
}

} // namespace webgpu
} // namespace mbgl
