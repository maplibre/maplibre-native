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
        : gfx::VertexAttribute(index_, dataType_, count_, /*stride_=*/0) {}
    VertexAttribute(const VertexAttribute& other)
        : gfx::VertexAttribute(other) {}
    VertexAttribute(VertexAttribute&& other)
        : gfx::VertexAttribute(std::move(other)) {}

public:
    ~VertexAttribute() override = default;

    // platform::GLenum getGLType() const { return glType; }
    // void setGLType(platform::GLenum value);

    // bool getNormalized() const { return normalized; }
    // void setNormalized(bool value) { normalized = value; }

    //std::size_t getStride() const;

    // static const std::vector<std::uint8_t>& getRaw(gfx::VertexAttribute& attr, platform::GLenum);

private:
    // static int getSize(platform::GLenum glType);
    // static int getStride(platform::GLenum glType);
    // static bool get(const gfx::VertexAttribute::ElementType&, platform::GLenum glType, uint8_t* buffer);

private:
    // platform::GLenum glType = 0;
    // bool normalized = false;
};

/// Stores a collection of vertex attributes by name
class VertexAttributeArray final : public gfx::VertexAttributeArray {
public:
    VertexAttributeArray(int initCapacity = 10)
        : gfx::VertexAttributeArray(initCapacity) {}
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
