#include <mbgl/gfx/vertex_attribute.hpp>

#include <mbgl/gfx/vertex_vector.hpp>

#include <algorithm>
#include <numeric>
#include <optional>

namespace mbgl {
namespace gfx {

std::size_t VertexAttribute::getCount() const {
    return sharedRawData ? sharedRawData->getRawCount() : items.size();
}

std::unique_ptr<VertexAttribute> VertexAttributeArray::nullref = nullptr;

VertexAttributeArray::VertexAttributeArray(int initCapacity)
    : attrs(initCapacity) {}

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
}

} // namespace gfx
} // namespace mbgl
