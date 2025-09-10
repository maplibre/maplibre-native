#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/texture2d.hpp>

namespace mbgl {
namespace webgpu {

class OffscreenTextureResource final : public RenderableResource {
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
    
    void swap() override {
        // WebGPU doesn't require explicit swap for offscreen textures
    }
    
    const RendererBackend& getBackend() const override {
        return context.getBackend();
    }
    
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
        bufferDesc.label = "Readback Buffer";
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
                WGPUImageCopyTexture src = {};
                src.texture = texture;
                src.mipLevel = 0;
                src.origin = {0, 0, 0};
                src.aspect = WGPUTextureAspect_All;
                
                WGPUImageCopyBuffer dst = {};
                dst.buffer = stagingBuffer;
                dst.layout.offset = 0;
                dst.layout.bytesPerRow = size.width * 4; // Assuming RGBA
                dst.layout.rowsPerImage = size.height;
                
                WGPUExtent3D copySize = {
                    static_cast<uint32_t>(size.width),
                    static_cast<uint32_t>(size.height),
                    1
                };
                
                wgpuCommandEncoderCopyTextureToBuffer(encoder, &src, &dst, &copySize);
                
                WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
                wgpuQueueSubmit(queue, 1, &commands);
                wgpuCommandBufferRelease(commands);
            }
            wgpuCommandEncoderRelease(encoder);
        }
        
        // Map buffer and read data (blocking)
        // Note: This is simplified - real implementation would need async callback
        wgpuBufferMapAsync(stagingBuffer, WGPUMapMode_Read, 0, dataSize,
                          [](WGPUBufferMapAsyncStatus status, void* userdata) {
                              // Callback for when mapping is complete
                              (void)status;
                              (void)userdata;
                          }, nullptr);
        
        // Wait for mapping to complete (simplified synchronous wait)
        wgpuDevicePoll(device, true, nullptr);
        
        // Read the mapped data
        const void* mappedData = wgpuBufferGetConstMappedRange(stagingBuffer, 0, dataSize);
        if (mappedData) {
            std::memcpy(data.get(), mappedData, dataSize);
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
        auto& resource = getResource<OffscreenTextureResource>();
        return resource.getTexture() != nullptr;
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