#pragma once

#include <mbgl/gfx/types.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/platform/gl_functions.hpp>

namespace mbgl {
namespace gfx {
class VertexBufferResource;
} // namespace gfx
namespace gl {

class UploadPass;

class VertexAttributeGL final : public gfx::VertexAttribute {
public:
    VertexAttributeGL(int index_, gfx::AttributeDataType dataType_, std::size_t count_)
        : VertexAttribute(index_, dataType_, count_) {}
    VertexAttributeGL(const VertexAttributeGL& other)
        : VertexAttribute(other),
          glType(other.glType) {}
    VertexAttributeGL(VertexAttributeGL&& other)
        : VertexAttribute(std::move(other)),
          glType(other.glType) {} // NOLINT(bugprone-use-after-move)
    ~VertexAttributeGL() override = default;

    platform::GLenum getGLType() const { return glType; }
    void setGLType(platform::GLenum value);

    bool getNormalized() const { return normalized; }
    void setNormalized(bool value) { normalized = value; }

    std::size_t getStride() const override;

    static const std::vector<std::uint8_t>& getRaw(gfx::VertexAttribute& attr,
                                                   platform::GLenum,
                                                   std::optional<std::chrono::duration<double>> lastUpdate);

private:
    static int getSize(platform::GLenum glType);
    static int getStride(platform::GLenum glType);
    static bool get(const gfx::VertexAttribute::ElementType&, platform::GLenum glType, uint8_t* buffer);

private:
    platform::GLenum glType = 0;
    bool normalized = false;
};

/// Stores a collection of vertex attributes by name
class VertexAttributeArrayGL final : public gfx::VertexAttributeArray {
public:
    VertexAttributeArrayGL() = default;
    VertexAttributeArrayGL(VertexAttributeArrayGL&& other)
        : VertexAttributeArray(std::move(other)) {}

    VertexAttributeArrayGL& operator=(VertexAttributeArrayGL&& other) {
        VertexAttributeArray::operator=(std::move(other));
        return *this;
    }
    VertexAttributeArrayGL& operator=(const VertexAttributeArrayGL&) = delete;

private:
    std::unique_ptr<gfx::VertexAttribute> create(int index,
                                                 gfx::AttributeDataType dataType,
                                                 std::size_t count) const override {
        return std::make_unique<VertexAttributeGL>(index, dataType, count);
    }

    std::unique_ptr<gfx::VertexAttribute> copy(const gfx::VertexAttribute& attr) const override {
        return std::make_unique<VertexAttributeGL>(static_cast<const VertexAttributeGL&>(attr));
    }
};

} // namespace gl
} // namespace mbgl
