#pragma once

#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/platform/gl_functions.hpp>

namespace mbgl {
namespace gl {

class VertexAttributeArrayGL;

class VertexAttributeGL final : public gfx::VertexAttribute {
    // Can only be created by VertexAttributeArrayGL
private:
    friend VertexAttributeArrayGL;
    VertexAttributeGL(int index_, gfx::AttributeDataType dataType_, int size_, std::size_t count_)
        : VertexAttribute(index_, dataType_, size_, count_, /*stride_=*/0) {}
    VertexAttributeGL(const VertexAttributeGL& other)
        : VertexAttribute(other),
          glType(other.glType) {}
    VertexAttributeGL(VertexAttributeGL&& other)
        : VertexAttribute(std::move(other)),
          glType(other.glType) {}

public:
    platform::GLenum getGLType() const { return glType; }
    void setGLType(platform::GLenum value);

    bool getNormalized() const { return normalized; }
    void setNormalized(bool value) { normalized = value; }

    std::size_t getStride() const;

    const std::vector<std::uint8_t>& getRaw(platform::GLenum) const;

private:
    static int getSize(platform::GLenum glType);
    static int getStride(platform::GLenum glType);
    static bool get(const gfx::VertexAttribute::ElementType&, platform::GLenum glType, uint8_t* buffer);

private:
    platform::GLenum glType = 0;
    mutable platform::GLenum rawType = 0;
    bool normalized = false;
};

/// Stores a collection of vertex attributes by name
class VertexAttributeArrayGL final : public gfx::VertexAttributeArray {
public:
    VertexAttributeArrayGL(int initCapacity = 10)
        : VertexAttributeArray(initCapacity) {}
    VertexAttributeArrayGL(VertexAttributeArrayGL&& other)
        : VertexAttributeArray(std::move(other)) {}
    VertexAttributeArrayGL(const VertexAttributeArrayGL&) = delete;

    VertexAttributeArrayGL& operator=(VertexAttributeArrayGL&& other) {
        VertexAttributeArray::operator=(std::move(other));
        return *this;
    }
    VertexAttributeArrayGL& operator=(const VertexAttributeArrayGL& other) {
        VertexAttributeArray::operator=(other);
        return *this;
    }

private:
    std::unique_ptr<gfx::VertexAttribute> create(int index,
                                                 gfx::AttributeDataType dataType,
                                                 int size,
                                                 std::size_t count) override {
        return std::unique_ptr<gfx::VertexAttribute>(new VertexAttributeGL(index, dataType, size, count));
    }
    std::unique_ptr<gfx::VertexAttribute> copy(const gfx::VertexAttribute& attr) override {
        return std::unique_ptr<gfx::VertexAttribute>(
            new VertexAttributeGL(static_cast<const VertexAttributeGL&>(attr)));
    }
};

} // namespace gl
} // namespace mbgl
