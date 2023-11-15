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
                                                                  const gfx::BufferUsageType usage) {
    if (!attrib_.getBuffer()) {
        auto& attrib = static_cast<VertexAttribute&>(attrib_);
        if (attrib.sharedRawData) {
            return uploadPass.getBuffer(attrib.sharedRawData, usage);
        } else {
            if (!attrib.rawData.empty()) {
                auto buffer = uploadPass.createVertexBufferResource(
                    attrib.rawData.data(), attrib.rawData.size(), usage, false);
                attrib.setBuffer(std::move(buffer));
                attrib.setRawData({});
            } else {
                assert(false);
            }
        }
    }
    return attrib_.getBuffer();
}

bool VertexAttributeArray::isDirty() const {
    return std::any_of(attrs.begin(), attrs.end(), [](const auto& kv) {
        if (kv.second) {
            // If we have shared data, the dirty flag from that overrides ours
            const auto& attrib = static_cast<const VertexAttribute&>(*kv.second);
            if (const auto& shared = attrib.getSharedRawData()) {
                return shared->getDirty();
            }
        }
        return kv.second && kv.second->isDirty();
    });
}

} // namespace mtl
} // namespace mbgl
