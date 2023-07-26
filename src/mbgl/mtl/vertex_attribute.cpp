#include <mbgl/mtl/vertex_attribute.hpp>

#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/mtl/buffer_resource.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/util/logging.hpp>

#include <cstring>
#include <sstream>

namespace mbgl {
namespace mtl {

//const std::vector<std::uint8_t>& VertexAttributeGL::getRaw(gfx::VertexAttribute& attr, platform::GLenum type) {
//    const auto count = attr.getCount();
//    const auto stride_ = getStride(type);
//    auto& rawData = attr.getRawData();
//    if (attr.isDirty() || rawData.size() != count * stride_) {
//        rawData.resize(stride_ * count);
//
//        if (!rawData.empty()) {
//            std::fill(rawData.begin(), rawData.end(), 0);
//
//            std::uint8_t* outPtr = rawData.data();
//            for (std::size_t i = 0; i < count; ++i) {
//                if (!get(attr.get(i), type, outPtr)) {
//                    // missing type conversion
//                    assert(false);
//                }
//                outPtr += stride_;
//            }
//        }
//        attr.setDirty(false);
//    }
//    return rawData;
//}

const gfx::UniqueVertexBufferResource& VertexAttribute::getBuffer(UploadPass& uploadPass, const gfx::BufferUsageType usage) {
    if (!buffer) {
        if (sharedRawData) {
            return uploadPass.getBuffer(sharedRawData, usage);
        } else if (!rawData.empty()) {
            buffer = uploadPass.createVertexBufferResource(rawData.data(), rawData.size(), usage);
        } else {
            // TODO: vertex building
            assert(false);
        }
    }
    return buffer;
}

} // namespace mtl
} // namespace mbgl
