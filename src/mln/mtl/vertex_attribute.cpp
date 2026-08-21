#include <mln/mtl/vertex_attribute.hpp>

#include <mln/gfx/vertex_vector.hpp>
#include <mln/mtl/buffer_resource.hpp>
#include <mln/mtl/upload_pass.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/convert.hpp>

#include <cstring>
#include <sstream>

namespace mln {
namespace mtl {

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

const std::unique_ptr<gfx::VertexAttribute>& VertexAttributeArray::set(const size_t id,
                                                                       int index,
                                                                       gfx::AttributeDataType dataType,
                                                                       int bufferIndex) {
    auto& attrib = gfx::VertexAttributeArray::set(id, index, dataType, 1);
    if (attrib) {
        static_cast<VertexAttribute*>(attrib.get())->setBufferIndex(bufferIndex);
    }
    return attrib;
}

} // namespace mtl
} // namespace mln
