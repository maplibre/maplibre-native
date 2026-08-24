#include <mln/webgpu/vertex_attribute.hpp>
#include <mln/gfx/vertex_vector.hpp>
#include <mln/webgpu/buffer_resource.hpp>
#include <mln/webgpu/upload_pass.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/convert.hpp>

#include <cstring>
#include <sstream>

namespace mln {
namespace webgpu {

const gfx::UniqueVertexBufferResource& VertexAttribute::getBuffer(gfx::VertexAttribute& attrib_,
                                                                  UploadPass& uploadPass,
                                                                  const gfx::BufferUsageType usage,
                                                                  bool forceUpdate) {
    if (!attrib_.getBuffer()) {
        auto& attrib = static_cast<VertexAttribute&>(attrib_);
        if (attrib.sharedRawData) {
            return uploadPass.getBuffer(attrib.sharedRawData, usage, forceUpdate);
        } else {
            if (!attrib.rawData.empty()) {
                auto buffer = uploadPass.createVertexBufferResource(
                    attrib.rawData.data(), attrib.rawData.size(), usage, false);
                attrib.setBuffer(std::move(buffer));
                attrib.setRawData({});
                attrib_.setDirty(false);
            } else {
                assert(false);
            }
        }
    }
    return attrib_.getBuffer();
}

bool VertexAttributeArray::isModifiedAfter(std::chrono::duration<double> time) const {
    // Use the base class implementation
    return gfx::VertexAttributeArray::isModifiedAfter(time);
}

} // namespace webgpu
} // namespace mln
