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

VertexAttributeArray& VertexAttributeArray::operator=(const VertexAttributeArray& other) {
    attrs.clear();
    for (const auto& kv : other.attrs) {
        if (kv.second) {
            add(kv.first, copy(*kv.second));
        }
    }
    return *this;
}

const std::unique_ptr<VertexAttribute>& VertexAttributeArray::get(const StringIdentity id) const {
    const auto result = attrs.find(id);
    return (result != attrs.end()) ? result->second : nullref;
}

const std::unique_ptr<VertexAttribute>& VertexAttributeArray::add(const StringIdentity id,
                                                                  int index,
                                                                  AttributeDataType dataType,
                                                                  std::size_t count) {
    const auto result = attrs.insert(std::make_pair(id, std::unique_ptr<VertexAttribute>()));
    if (result.second) {
        result.first->second = create(index, dataType, count);
        return result.first->second;
    } else {
        return nullref;
    }
}

const std::unique_ptr<VertexAttribute>& VertexAttributeArray::getOrAdd(const StringIdentity id,
                                                                       int index,
                                                                       AttributeDataType dataType,
                                                                       std::size_t count) {
    const auto result = attrs.insert(std::make_pair(id, std::unique_ptr<VertexAttribute>()));
    if (auto& attr = result.first->second; result.second) {
        return attr = create(index, dataType, count);
    } else if ((dataType == AttributeDataType::Invalid || attr->getDataType() == dataType) &&
               (index < 0 || attr->getIndex() == index) && (count == 0 || attr->getCount() == count)) {
        return attr;
    }
    return nullref;
}

std::size_t VertexAttributeArray::getTotalSize() const {
    return std::accumulate(attrs.begin(), attrs.end(), std::size_t(0), [](const auto acc, const auto& kv) {
        return acc + kv.second->getStride();
    });
}

std::size_t VertexAttributeArray::getMaxCount() const {
    return std::accumulate(attrs.begin(), attrs.end(), std::size_t(0), [](const auto acc, const auto& kv) {
        return std::max(acc, kv.second->getCount());
    });
}

void VertexAttributeArray::clear() {
    attrs.clear();
}

void VertexAttributeArray::resolve(const VertexAttributeArray& overrides, ResolveDelegate delegate) const {
    for (auto& kv : attrs) {
        delegate(kv.first, *kv.second, overrides.get(kv.first));
    }
    // For OpenGL, the shader attributes are established with reflection, and we have extra
    // entries when we share attributes between, e.g., fill and fill-outline drawables.
#if !defined(NDEBUG) && MLN_RENDERER_BACKEND_METAL
    // Every override should match a defined attribute.
    for (const auto& kv : overrides.attrs) {
        const auto hit = attrs.find(kv.first);
        assert(hit != attrs.end());
    }
#endif
}

const UniqueVertexAttribute& VertexAttributeArray::add(const StringIdentity id,
                                                       std::unique_ptr<VertexAttribute>&& attr) {
    const auto result = attrs.insert(std::make_pair(id, std::unique_ptr<VertexAttribute>()));
    if (result.second) {
        result.first->second = std::move(attr);
        return result.first->second;
    } else {
        return nullref;
    }
}

} // namespace gfx
} // namespace mbgl
