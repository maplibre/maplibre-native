#pragma once

#include <mbgl/gfx/vertex_attribute.hpp>

namespace mbgl {
namespace webgpu {

class VertexAttributeArray final : public gfx::VertexAttributeArray {
public:
    VertexAttributeArray() = default;
    VertexAttributeArray(VertexAttributeArray&& other)
        : gfx::VertexAttributeArray(std::move(other)) {}
    
    VertexAttributeArray& operator=(VertexAttributeArray&& other) {
        gfx::VertexAttributeArray::operator=(std::move(other));
        return *this;
    }
    
    VertexAttributeArray& operator=(const VertexAttributeArray& other) = delete;
    VertexAttributeArray(const VertexAttributeArray&) = delete;
};

} // namespace webgpu
} // namespace mbgl