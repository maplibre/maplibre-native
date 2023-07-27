#include <mbgl/mtl/vertex_attribute.hpp>

#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/mtl/buffer_resource.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/util/logging.hpp>

#include <cstring>
#include <sstream>

namespace mbgl {
namespace mtl {

std::size_t VertexAttribute::getStrideOf(gfx::AttributeDataType type) {
    switch (type) {
        case gfx::AttributeDataType::Byte:
            return 1;
        case gfx::AttributeDataType::Byte2:
            return 2;
        case gfx::AttributeDataType::Byte3:
            return 3;
        case gfx::AttributeDataType::Byte4:
            return 4;
        case gfx::AttributeDataType::UByte:
            return 1;
        case gfx::AttributeDataType::UByte2:
            return 2;
        case gfx::AttributeDataType::UByte3:
            return 3;
        case gfx::AttributeDataType::UByte4:
            return 4;
        case gfx::AttributeDataType::Short:
            return 2;
        case gfx::AttributeDataType::Short2:
            return 4;
        case gfx::AttributeDataType::Short3:
            return 6;
        case gfx::AttributeDataType::Short4:
            return 8;
        case gfx::AttributeDataType::UShort:
            return 2;
        case gfx::AttributeDataType::UShort2:
            return 4;
        case gfx::AttributeDataType::UShort3:
            return 6;
        case gfx::AttributeDataType::UShort4:
            return 8;
        case gfx::AttributeDataType::UShort8:
            return 16;
        case gfx::AttributeDataType::Int:
            return 4;
        case gfx::AttributeDataType::Int2:
            return 8;
        case gfx::AttributeDataType::Int3:
            return 12;
        case gfx::AttributeDataType::Int4:
            return 16;
        case gfx::AttributeDataType::UInt:
            return 4;
        case gfx::AttributeDataType::UInt2:
            return 8;
        case gfx::AttributeDataType::UInt3:
            return 12;
        case gfx::AttributeDataType::UInt4:
            return 16;
        case gfx::AttributeDataType::Float:
            return 4;
        case gfx::AttributeDataType::Float2:
            return 8;
        case gfx::AttributeDataType::Float3:
            return 12;
        case gfx::AttributeDataType::Float4:
            return 16;
        default:
            assert(false);
            return 0;
    }
}

const gfx::UniqueVertexBufferResource& VertexAttribute::getBuffer(gfx::VertexAttribute& attrib_,
                                                                  UploadPass& uploadPass,
                                                                  const gfx::BufferUsageType usage) {
    if (!attrib_.getBuffer()) {
        auto& attrib = static_cast<VertexAttribute&>(attrib_);
        if (attrib.sharedRawData) {
            return uploadPass.getBuffer(attrib.sharedRawData, usage);
        } else if (!attrib.rawData.empty()) {
            auto buffer = uploadPass.createVertexBufferResource(attrib.rawData.data(), attrib.rawData.size(), usage);
            attrib.setBuffer(std::move(buffer));
            attrib.setRawData({});
        } else {
            // TODO: vertex building
            assert(false);
        }
    }
    return attrib_.getBuffer();
}

} // namespace mtl
} // namespace mbgl
