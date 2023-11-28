#pragma once

#include <mbgl/gfx/types.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>

namespace mbgl {
namespace gfx {
class VertexBufferResource;
} // namespace gfx
namespace mtl {

class VertexAttributeArray;
class UploadPass;

class VertexAttribute final : public gfx::VertexAttribute {
    // Can only be created by VertexAttributeArray
private:
    friend VertexAttributeArray;
    VertexAttribute(int index_, gfx::AttributeDataType dataType_, std::size_t count_)
        : gfx::VertexAttribute(index_, dataType_, count_) {}
    VertexAttribute(const VertexAttribute& other) = delete;
    VertexAttribute(VertexAttribute&& other)
        : gfx::VertexAttribute(std::move(other)) {}

public:
    ~VertexAttribute() override = default;

    static const gfx::UniqueVertexBufferResource& getBuffer(gfx::VertexAttribute&,
                                                            UploadPass&,
                                                            const gfx::BufferUsageType);
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

    /// Indicates whether any values have changed
    bool isDirty() const override;

private:
    gfx::UniqueVertexAttribute create(int index, gfx::AttributeDataType dataType, std::size_t count) const override {
        return gfx::UniqueVertexAttribute(new VertexAttribute(index, dataType, count));
    }
};

} // namespace mtl
} // namespace mbgl
