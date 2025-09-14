#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/util/logging.hpp>
#include <cstring>

namespace mbgl {
namespace webgpu {

class OffscreenTextureResource final : public gfx::RenderableResource {
public:
    OffscreenTextureResource(Context& context_,
                             const Size size_,
                             const gfx::TextureChannelDataType type_,
                             bool depth,
                             bool stencil)
        : context(context_),
          size(size_),
          type(type_) {
        assert(!size.isEmpty());
        
        // Create color texture
        colorTexture = context.createTexture2D();
        colorTexture->setSize(size);
        colorTexture->setFormat(gfx::TexturePixelType::RGBA, type);
        colorTexture->setSamplerConfiguration(
            {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        
        // Create depth texture if needed
        if (depth) {
            depthTexture = context.createTexture2D();
            depthTexture->setSize(size);
            depthTexture->setFormat(gfx::TexturePixelType::Depth, gfx::TextureChannelDataType::Float);
            depthTexture->setSamplerConfiguration(
                {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        }
        
        // Create stencil texture if needed
        if (stencil) {
            stencilTexture = context.createTexture2D();
            stencilTexture->setSize(size);
            stencilTexture->setFormat(gfx::TexturePixelType::Stencil, gfx::TextureChannelDataType::UnsignedByte);
            stencilTexture->setSamplerConfiguration(
                {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
        }
    }
    
    ~OffscreenTextureResource() noexcept override {
        context.renderingStats().numFrameBuffers--;
    }
    
    void bind() override {
        // Create textures if not already created
        colorTexture->create();
        
        if (depthTexture) {
            depthTexture->create();
        }
        
        if (stencilTexture) {
            stencilTexture->create();
        }
    }
    
    // Note: swap() and getBackend() are not part of gfx::RenderableResource
    // They would be implemented if we had a WebGPU-specific RenderableResource base class
    
    PremultipliedImage readStillImage() {
        // Read back the color texture data
        auto dataSize = colorTexture->getDataSize();
        auto data = std::make_unique<uint8_t[]>(dataSize);
        
        // WebGPU texture readback requires:
        // 1. Create a buffer with MAP_READ usage
        // 2. Copy texture to buffer using command encoder
        // 3. Map the buffer and read the data
        // This is an async operation in WebGPU, but we need sync behavior here
        
        auto& backend = static_cast<const RendererBackend&>(context.getBackend());
        auto device = static_cast<WGPUDevice>(backend.getDevice());
        auto queue = static_cast<WGPUQueue>(backend.getQueue());
        if (!device || !queue) {
            return {size, std::move(data)};
        }
        
        // Create staging buffer for readback
        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView bufferLabel = {"Readback Buffer", strlen("Readback Buffer")};
        bufferDesc.label = bufferLabel;
        bufferDesc.size = dataSize;
        bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
        
        WGPUBuffer stagingBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        if (!stagingBuffer) {
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
                dst.layout.bytesPerRow = size.width * 4; // Assuming RGBA
                dst.layout.rowsPerImage = size.height;
                
                WGPUExtent3D copySize = {
                    static_cast<uint32_t>(size.width),
                    static_cast<uint32_t>(size.height),
                    1
                };
                
                // Copy texture to buffer - function name varies between Dawn and wgpu
                // Using the standard C API name
                wgpuCommandEncoderCopyTextureToBuffer(encoder, &src, &dst, &copySize);
                
                WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
                wgpuQueueSubmit(queue, 1, &commands);
                wgpuCommandBufferRelease(commands);
            }
            wgpuCommandEncoderRelease(encoder);
        }
        
        // Map buffer and read data (blocking)
        // Use Dawn's new async API with WGPUBufferMapCallbackInfo
        struct MapContext {
            bool completed = false;
            WGPUMapAsyncStatus status = WGPUMapAsyncStatus_Error;
        };
        MapContext mapContext;
        
        WGPUBufferMapCallbackInfo callbackInfo = {};
        callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
        callbackInfo.callback = [](WGPUMapAsyncStatus status, WGPUStringView message, void* userdata1, void* userdata2) {
            auto* ctx = static_cast<MapContext*>(userdata1);
            ctx->completed = true;
            ctx->status = status;
            (void)message;
            (void)userdata2;
        };
        callbackInfo.userdata1 = &mapContext;
        callbackInfo.userdata2 = nullptr;
        
        WGPUFuture future = wgpuBufferMapAsync(stagingBuffer, WGPUMapMode_Read, 0, dataSize, callbackInfo);
        
        // Proper async handling with wgpuInstanceWaitAny
        WGPUInstance instance = static_cast<WGPUInstance>(backend.getInstance());
        if (instance) {
            WGPUFutureWaitInfo waitInfo = {};
            waitInfo.future = future;
            waitInfo.completed = false;
            
            // Wait for the async operation with a 5 second timeout
            uint64_t timeout_ns = 5000000000; // 5 seconds in nanoseconds
            WGPUWaitStatus waitStatus = wgpuInstanceWaitAny(instance, 1, &waitInfo, timeout_ns);
            
            if (waitStatus == WGPUWaitStatus_Success) {
                // The callback should have been invoked
                if (!mapContext.completed) {
                }
            } else if (waitStatus == WGPUWaitStatus_TimedOut) {
            } else {
            }
        } else {
        }
        
        // Check if mapping was successful and read the data
        if (mapContext.completed && mapContext.status == WGPUMapAsyncStatus_Success) {
            const void* mappedData = wgpuBufferGetConstMappedRange(stagingBuffer, 0, dataSize);
            if (mappedData) {
                std::memcpy(data.get(), mappedData, dataSize);
            }
        }
        
        wgpuBufferUnmap(stagingBuffer);
        wgpuBufferRelease(stagingBuffer);
        
        return {size, std::move(data)};
    }
    
    gfx::Texture2DPtr& getTexture() {
        assert(colorTexture);
        return colorTexture;
    }
    
private:
    Context& context;
    const Size size;
    const gfx::TextureChannelDataType type;
    gfx::Texture2DPtr colorTexture;
    gfx::Texture2DPtr depthTexture;
    gfx::Texture2DPtr stencilTexture;
};

OffscreenTexture::OffscreenTexture(Context& context,
                                   const Size size_,
                                   const gfx::TextureChannelDataType type,
                                   bool depth,
                                   bool stencil)
    : gfx::OffscreenTexture(size_, std::make_unique<OffscreenTextureResource>(context, size_, type, depth, stencil)) {
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