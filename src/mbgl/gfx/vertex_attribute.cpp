#include <mbgl/gfx/vertex_attribute.hpp>

#include <mbgl/gfx/vertex_vector.hpp>

#include <algorithm>
#include <numeric>
#include <optional>

namespace mbgl {
namespace gfx {

namespace {
const UniqueVertexAttribute nullref;
} // namespace

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
            return 0;
    }
}

std::size_t VertexAttribute::getCount() const {
    return sharedRawData ? sharedRawData->getRawCount() : items.size();
}

const std::unique_ptr<VertexAttribute> VertexAttributeArray::nullref;
const std::string VertexAttributeArray::attributePrefix = "a_";

VertexAttributeArray::VertexAttributeArray(VertexAttributeArray&& other)
    : attrs(std::move(other.attrs)) {}

VertexAttributeArray& VertexAttributeArray::operator=(VertexAttributeArray&& other) {
    attrs = std::move(other.attrs);
    return *this;
}

const std::unique_ptr<VertexAttribute>& VertexAttributeArray::get(const size_t id) const {
    return (id < attrs.size()) ? attrs[id] : nullref;
}

const std::unique_ptr<VertexAttribute>& VertexAttributeArray::set(const size_t id,
                                                                  int index,
                                                                  AttributeDataType dataType,
                                                                  std::size_t count) {
    assert(id < attrs.size());
    if (id >= attrs.size()) {
        return nullref;
    }
    attrs[id] = create(index, dataType, count);
    return attrs[id];
}

std::size_t VertexAttributeArray::getTotalSize() const {
    return std::accumulate(attrs.begin(), attrs.end(), std::size_t(0), [](const auto acc, const auto& attr) {
        return acc + (attr ? attr->getStride() : 0);
    });
}

std::size_t VertexAttributeArray::getMaxCount() const {
    return std::accumulate(attrs.begin(), attrs.end(), std::size_t(0), [](const auto acc, const auto& attr) {
        return std::max(acc, (attr ? attr->getCount() : 0));
    });
}

void VertexAttributeArray::clear() {
    for (auto& attr : attrs) {
        attr = nullptr;
    }
}

} // namespace gfx
} // namespace mbgl
