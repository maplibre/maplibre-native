#include <mbgl/webgpu/vertex_attribute.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>

namespace mbgl {
namespace webgpu {

const gfx::UniqueVertexBufferResource& VertexAttribute::getBuffer(gfx::VertexAttribute& attrib,
                                                                  UploadPass& uploadPass,
                                                                  const gfx::BufferUsageType usage,
                                                                  bool forceUpdate) {
    if (!attrib.getBuffer() || forceUpdate) {
        // Create or update the vertex buffer
        const auto& data = attrib.getData();
        if (!data.empty()) {
            attrib.setBuffer(uploadPass.createVertexBufferResource(
                data.data(), 
                data.size(),
                usage));
        }
    }
    return attrib.getBuffer();
}

} // namespace webgpu
} // namespace mbgl