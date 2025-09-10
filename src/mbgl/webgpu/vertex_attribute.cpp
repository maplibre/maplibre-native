#include <mbgl/webgpu/vertex_attribute.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>

namespace mbgl {
namespace webgpu {

const gfx::UniqueVertexBufferResource& VertexAttribute::getBuffer(gfx::VertexAttribute& attrib_,
                                                                  UploadPass& uploadPass,
                                                                  const gfx::BufferUsageType usage,
                                                                  bool forceUpdate) {
    if (!attrib_.getBuffer() || forceUpdate) {
        auto& attrib = static_cast<VertexAttribute&>(attrib_);
        
        // Check if we have shared raw data
        if (attrib.sharedRawData) {
            // WebGPU doesn't have a getBuffer method yet, need to create buffer
            if (!attrib.rawData.empty()) {
                auto buffer = uploadPass.createVertexBufferResource(
                    attrib.rawData.data(), 
                    attrib.rawData.size(),
                    usage,
                    false);
                attrib.setBuffer(std::move(buffer));
                attrib.setRawData({});
                attrib_.setDirty(false);
            }
        } else {
            // Check if we have raw data to upload
            if (!attrib.rawData.empty()) {
                auto buffer = uploadPass.createVertexBufferResource(
                    attrib.rawData.data(), 
                    attrib.rawData.size(),
                    usage,
                    false);
                attrib.setBuffer(std::move(buffer));
                attrib.setRawData({});
                attrib_.setDirty(false);
            }
        }
    }
    return attrib_.getBuffer();
}

} // namespace webgpu
} // namespace mbgl