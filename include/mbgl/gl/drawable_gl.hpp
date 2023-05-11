#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/util/color.hpp>

#include <memory>

namespace mbgl {
namespace gfx {

class IndexBuffer;
class VertexBufferResource;

using UniqueVertexBufferResource = std::unique_ptr<gfx::VertexBufferResource>;

} // namespace gfx

namespace gl {

class VertexArray;

class DrawableGL : public gfx::Drawable {
public:
    DrawableGL(std::string name);
    ~DrawableGL() override;

    void draw(const PaintParameters&) const override;

    void setIndexData(std::vector<uint16_t> indexes, std::size_t indexOffset = 0, std::size_t indexLength = 0);
    std::vector<std::uint16_t>& getIndexData() const override;

    const gfx::VertexAttributeArray& getVertexAttributes() const override;
    void setVertexAttributes(const gfx::VertexAttributeArray& value) override;
    void setVertexAttributes(gfx::VertexAttributeArray&& value) override;

    gfx::VertexAttributeArray& mutableVertexAttributes();

    const gl::VertexArray& getVertexArray() const;
    void setVertexArray(gl::VertexArray&&, gfx::UniqueVertexBufferResource&&, gfx::IndexBuffer&&);

    const gfx::UniqueVertexBufferResource& getBuffer() const;
    const gfx::IndexBuffer& getIndexBuffer() const;

    const gfx::UniformBufferArray& getUniformBuffers() const override;
    gfx::UniformBufferArray& mutableUniformBuffers() override;

    /// Reset a single color attribute for all vertexes
    void resetColor(const Color&) override;

    static gfx::VertexAttribute::float4 colorAttrValue(const Color& color) {
        const auto components = color.toArray();
        return {static_cast<float>(components[0] / 255.0),
                static_cast<float>(components[1] / 255.0),
                static_cast<float>(components[2] / 255.0),
                static_cast<float>(components[3])};
    }

protected:
    class Impl;
    const std::unique_ptr<Impl> impl;

    // For testing only.
    DrawableGL(std::unique_ptr<Impl>);

private:
    void bindUniformBuffers() const;
    void unbindUniformBuffers() const;
};

} // namespace gl
} // namespace mbgl
