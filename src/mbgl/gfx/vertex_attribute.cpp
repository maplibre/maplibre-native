#include <mbgl/gfx/vertex_attribute.hpp>

namespace mbgl {
namespace gfx {

VertexAttributeArray::VertexAttributeArray(int initCapacity)
    : attrs(initCapacity) {
}

VertexAttributeArray::VertexAttributeArray(VertexAttributeArray &&other)
    : attrs(std::move(other.attrs)) {
}

VertexAttributeArray::VertexAttributeArray(const VertexAttributeArray& other)
    : attrs(other.attrs.size()) {
    operator=(other);
}

VertexAttributeArray& VertexAttributeArray::operator=(VertexAttributeArray &&other) {
    attrs = std::move(other.attrs);
    return *this;
}

VertexAttributeArray& VertexAttributeArray::operator=(const VertexAttributeArray& other) {
    attrs.clear();
    for (const auto &kv : other.attrs) {
        add(kv.first, copy(*kv.second));
    }
    return *this;
}

} // namespace gfx
} // namespace mbgl
