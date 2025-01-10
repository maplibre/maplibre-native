#include <mbgl/mtl/vertex_attribute.hpp>

#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/mtl/buffer_resource.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/convert.hpp>

#include <cstring>
#include <sstream>

namespace mbgl {
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

} // namespace mtl
} // namespace mbgl
