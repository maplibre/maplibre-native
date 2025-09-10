#pragma once

#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <memory>
#include <vector>

namespace mbgl {
namespace webgpu {

class CommandEncoder;
class Context;

class UploadPass final : public gfx::UploadPass {
public:
    UploadPass(CommandEncoder& commandEncoder, const char* name, gfx::UploadPassDescriptor&& descriptor);
    ~UploadPass() override;

    void upload(gfx::Drawable&) override;
    void uploadTexture(gfx::Texture2D&) override;
    void updateUniformBuffer(gfx::UniformBuffer&, const void* data, std::size_t size) override;
    void updateTexture(gfx::Texture2D&, const void* data, std::size_t size) override;
    
private:
    CommandEncoder& commandEncoder;
    gfx::UploadPassDescriptor descriptor;
    
    // Staging buffers for uploads
    struct StagingBuffer {
        WGPUBuffer buffer = nullptr;
        std::size_t size = 0;
        void* mappedData = nullptr;
    };
    std::vector<StagingBuffer> stagingBuffers;
    
    StagingBuffer* getOrCreateStagingBuffer(std::size_t size);
    void copyBufferToBuffer(WGPUBuffer src, WGPUBuffer dst, std::size_t size);
    void copyBufferToTexture(WGPUBuffer src, WGPUTexture dst, const WGPUExtent3D& size);
};

} // namespace webgpu
} // namespace mbgl