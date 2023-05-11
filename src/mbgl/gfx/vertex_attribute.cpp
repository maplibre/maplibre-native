#include <mbgl/gfx/vertex_attribute.hpp>

#include <algorithm>
#include <numeric>
#include <optional>

namespace mbgl {
namespace gfx {

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
        add(kv.first, copy(*kv.second));
    }
    return *this;
}

const std::unique_ptr<VertexAttribute>& VertexAttributeArray::get(const std::string& name) const {
    const auto result = attrs.find(name);
    return (result != attrs.end()) ? result->second : nullref;
}

const std::unique_ptr<VertexAttribute>& VertexAttributeArray::add(
    std::string name, int index, AttributeDataType dataType, int size, std::size_t count) {
    const auto result = attrs.insert(std::make_pair(std::move(name), std::unique_ptr<VertexAttribute>()));
    if (result.second) {
        result.first->second = create(index, dataType, size, count);
        return result.first->second;
    } else {
        return nullref;
    }
}

const std::unique_ptr<VertexAttribute>& VertexAttributeArray::getOrAdd(
    std::string name, int index, AttributeDataType dataType, int size, std::size_t count) {
    // attrs.emplace_back(std::make_unique<VertexAttribute>(dataType, count));
    const auto result = attrs.insert(std::make_pair(std::move(name), std::unique_ptr<VertexAttribute>()));
    if (result.second) {
        result.first->second = create(index, dataType, size, count);
    } else if (result.first->second->getDataType() != dataType ||
               result.first->second->getSize() != (std::size_t)size || result.first->second->getCount() != count) {
        return nullref;
    }
    return result.first->second;
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

void VertexAttributeArray::resolve(const VertexAttributeArray& overrides, ResolveDelegate delegate) const {
    for (auto& kv : attrs) {
        delegate(kv.first, *kv.second, overrides.get(kv.first));
    }
}

} // namespace gfx
} // namespace mbgl
