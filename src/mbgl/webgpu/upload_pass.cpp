#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/buffer_resource.hpp>
#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <mbgl/webgpu/index_buffer_resource.hpp>

namespace mbgl {
namespace webgpu {

UploadPass::UploadPass(CommandEncoder& commandEncoder_, const char* name)
    : commandEncoder(commandEncoder_) {
    pushDebugGroup(name);
}

void UploadPass::pushDebugGroup(const char* /*name*/) {
    // WebGPU debug groups are typically set on command encoders
    // This would require access to the WebGPU command encoder
    // For now, this is a no-op
}

void UploadPass::popDebugGroup() {
    // WebGPU debug groups are typically set on command encoders
    // This would require access to the WebGPU command encoder
    // For now, this is a no-op
}

gfx::Context& UploadPass::getContext() {
    return commandEncoder.getContext();
}

const gfx::Context& UploadPass::getContext() const {
    return commandEncoder.getContext();
}

std::unique_ptr<gfx::VertexBufferResource> UploadPass::createVertexBufferResource(
    const void* data,
    std::size_t size,
    gfx::BufferUsageType /*usage*/,
    bool persistent) {

    auto& context = static_cast<Context&>(getContext());
    BufferResource buffer(context, data, size, WGPUBufferUsage_Vertex, /*isIndexBuffer=*/false, persistent);
    return std::make_unique<VertexBufferResource>(std::move(buffer));
}

void UploadPass::updateVertexBufferResource(gfx::VertexBufferResource& resource, 
                                           const void* data, 
                                           std::size_t size) {
    auto& buffer = static_cast<VertexBufferResource&>(resource);
    buffer.update(data, size);
}

std::unique_ptr<gfx::IndexBufferResource> UploadPass::createIndexBufferResource(
    const void* data,
    std::size_t size,
    gfx::BufferUsageType /*usage*/,
    bool persistent) {

    auto& context = static_cast<Context&>(getContext());
    BufferResource buffer(context, data, size, WGPUBufferUsage_Index, /*isIndexBuffer=*/true, persistent);
    return std::make_unique<IndexBufferResource>(std::move(buffer));
}

void UploadPass::updateIndexBufferResource(gfx::IndexBufferResource& resource, 
                                          const void* data, 
                                          std::size_t size) {
    auto& buffer = static_cast<IndexBufferResource&>(resource);
    buffer.update(data, size);
}

gfx::AttributeBindingArray UploadPass::buildAttributeBindings(
    const std::size_t /*vertexCount*/,
    const gfx::AttributeDataType /*vertexType*/,
    const std::size_t /*vertexAttributeIndex*/,
    const std::vector<std::uint8_t>& vertexData,
    const gfx::VertexAttributeArray& /*defaults*/,
    const gfx::VertexAttributeArray& /*overrides*/,
    gfx::BufferUsageType usage,
    const std::optional<std::chrono::duration<double>> /*lastUpdate*/,
    /*out*/ std::vector<std::unique_ptr<gfx::VertexBufferResource>>& outBuffers) {
    
    gfx::AttributeBindingArray result;
    
    // Create vertex buffer for the vertex data
    if (!vertexData.empty()) {
        auto vertexBuffer = createVertexBufferResource(
            vertexData.data(),
            vertexData.size(),
            usage,
            false // not persistent for now
        );
        
        // Build attribute bindings based on the vertex attributes
        // This is a simplified implementation - a full implementation would
        // need to properly parse the vertex format and create appropriate bindings
        
        outBuffers.push_back(std::move(vertexBuffer));
    }
    
    return result;
}

} // namespace webgpu
} // namespace mbgl