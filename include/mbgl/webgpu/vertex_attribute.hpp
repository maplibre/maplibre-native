#pragma once

#include <mbgl/gfx/types.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>

namespace mbgl {
namespace gfx {
class VertexBufferResource;
} // namespace gfx
namespace webgpu {

class UploadPass;

class VertexAttribute final : public gfx::VertexAttribute {
public:
    VertexAttribute(int index_, gfx::AttributeDataType dataType_, std::size_t count_)
        : gfx::VertexAttribute(index_, dataType_, count_) {}
    VertexAttribute(const VertexAttribute& other) = delete;
    VertexAttribute(VertexAttribute&& other)
        : gfx::VertexAttribute(std::move(other)) {}
    ~VertexAttribute() override = default;

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

    /// Indicates whether any values have changed
    bool isModifiedAfter(std::chrono::duration<double> time) const;

private:
    gfx::UniqueVertexAttribute create(int index, gfx::AttributeDataType dataType, std::size_t count) const override {
        return std::make_unique<VertexAttribute>(index, dataType, count);
    }
};

} // namespace webgpu
} // namespace mbgl
