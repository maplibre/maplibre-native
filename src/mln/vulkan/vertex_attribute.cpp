#include <mln/vulkan/vertex_attribute.hpp>

#include <mln/gfx/vertex_vector.hpp>
#include <mln/vulkan/buffer_resource.hpp>
#include <mln/vulkan/vertex_buffer_resource.hpp>
#include <mln/vulkan/upload_pass.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/convert.hpp>

#include <cstring>
#include <sstream>

namespace mln {
namespace vulkan {

size_t VertexAttribute::getBufferUsage() const {
    if (ubo == -1) {
        return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
}

const gfx::UniqueVertexBufferResource& VertexAttribute::getBuffer(gfx::VertexAttribute& attrib_,
                                                                  UploadPass& uploadPass,
                                                                  size_t usage,
                                                                  bool forceUpdate) {
    if (!attrib_.getBuffer()) {
        auto& attrib = static_cast<VertexAttribute&>(attrib_);
        if (attrib.sharedRawData) {
            return uploadPass.getBuffer(attrib.sharedRawData, usage, forceUpdate);
        } else {
            if (!attrib.rawData.empty()) {
                auto buffer = std::make_unique<VertexBufferResource>(
                    uploadPass.createBufferResource(attrib.rawData.data(), attrib.rawData.size(), usage, false));
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

} // namespace vulkan
} // namespace mln
