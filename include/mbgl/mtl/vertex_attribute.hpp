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
    VertexAttribute(const VertexAttribute& other)
        : gfx::VertexAttribute(other) {}
    VertexAttribute(VertexAttribute&& other)
        : gfx::VertexAttribute(std::move(other)) {}

public:
    ~VertexAttribute() override = default;

    /// Get the Metal buffer, creating it if necessary
    // const gfx::UniqueVertexBufferResource& getBuffer(UploadPass&, const gfx::BufferUsageType);

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
    VertexAttributeArray& operator=(const VertexAttributeArray& other) {
        gfx::VertexAttributeArray::operator=(other);
        return *this;
    }

    gfx::UniqueVertexAttributeArray clone() const override {
        auto newAttrs = std::make_unique<VertexAttributeArray>();
        newAttrs->copy(*this);
        return newAttrs;
    }

private:
    gfx::UniqueVertexAttribute create(int index, gfx::AttributeDataType dataType, std::size_t count) const override {
        return gfx::UniqueVertexAttribute(new VertexAttribute(index, dataType, count));
    }

    using gfx::VertexAttributeArray::copy;

    gfx::UniqueVertexAttribute copy(const gfx::VertexAttribute& attr) const override {
        return gfx::UniqueVertexAttribute(new VertexAttribute(static_cast<const VertexAttribute&>(attr)));
    }
};

} // namespace mtl
} // namespace mbgl
