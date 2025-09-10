#pragma once

#include <mbgl/gfx/vertex_attribute.hpp>

namespace mbgl {
namespace gfx {
class VertexBufferResource;
} // namespace gfx
namespace webgpu {

class UploadPass;

class VertexAttribute final : public gfx::VertexAttribute {
public:
    using gfx::VertexAttribute::VertexAttribute;
    
    static const gfx::UniqueVertexBufferResource& getBuffer(gfx::VertexAttribute&,
                                                            UploadPass&,
                                                            const gfx::BufferUsageType,
                                                            bool forceUpdate);
};

/// Stores a collection of vertex attributes by name
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

private:
    gfx::UniqueVertexAttribute create(int index, gfx::AttributeDataType dataType, std::size_t count) const override {
        return std::make_unique<VertexAttribute>(index, dataType, count);
    }
};

} // namespace webgpu
} // namespace mbgl