#pragma once

#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/platform/gl_functions.hpp>

namespace mbgl {
namespace gl {

class VertexAttributeArrayGL;

class VertexAttributeGL final : public gfx::VertexAttribute {
    // Can only be created by VertexAttributeArrayGL
private:
    friend VertexAttributeArrayGL;
    VertexAttributeGL(int index_, gfx::AttributeDataType dataType_, std::size_t count_)
    : VertexAttribute(index_, dataType_, count_) {
    }
    VertexAttributeGL(const VertexAttributeGL& other)
    : VertexAttribute(other),
    glType(other.glType) {
    }
    VertexAttributeGL(VertexAttributeGL&& other)
    : VertexAttribute(std::move(other)),
    glType(other.glType) {
    }
    
public:
    platform::GLenum getGLType() const { return glType; }
    void setGLType(platform::GLenum value) { glType = value; }

    bool getNormalized() const { return normalized; }
    void setNormalized(bool value) { normalized = value; }

    std::size_t getStride() const { return stride; }
    const std::vector<uint8_t>& getRaw() const;

private:
    platform::GLenum glType = 0;
    bool normalized = false;
    mutable int stride = 0;
    mutable std::vector<uint8_t> rawData;
};

/// Stores a collection of vertex attributes by name
class VertexAttributeArrayGL final : public gfx::VertexAttributeArray {
public:
    VertexAttributeArrayGL(int initCapacity = 10) : VertexAttributeArray(initCapacity) { }
    VertexAttributeArrayGL(VertexAttributeArrayGL &&other) : VertexAttributeArray(std::move(other)) { }
    VertexAttributeArrayGL(const VertexAttributeArrayGL& other) : VertexAttributeArray(other) { }
    ~VertexAttributeArrayGL() = default;
    
    VertexAttributeArrayGL& operator=(VertexAttributeArrayGL &&other) {
        VertexAttributeArray::operator=(std::move(other));
        return *this;
    }
    VertexAttributeArrayGL& operator=(const VertexAttributeArrayGL& other) {
        VertexAttributeArray::operator=(other);
        return *this;
    }
    
private:
    std::unique_ptr<gfx::VertexAttribute> create(int index, gfx::AttributeDataType dataType, std::size_t count) override {
        return std::unique_ptr<gfx::VertexAttribute>(new VertexAttributeGL(index, dataType, count));
    }
    std::unique_ptr<gfx::VertexAttribute> copy(const gfx::VertexAttribute& attr) override {
        return std::unique_ptr<gfx::VertexAttribute>(new VertexAttributeGL(static_cast<const VertexAttributeGL&>(attr)));
    }
};

} // namespace gfx
} // namespace mbgl
