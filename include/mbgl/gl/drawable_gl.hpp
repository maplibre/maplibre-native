#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>

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
    DrawableGL();
    virtual ~DrawableGL();

    void draw(const PaintParameters &) const override;

    void setVertData(std::vector<std::uint8_t> data) { vertData = std::move(data); }

    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }
    void setVertexAttributes(const gfx::VertexAttributeArray& value) override {
        vertexAttributes = static_cast<const VertexAttributeArrayGL&>(value);
    }
    void setVertexAttributes(gfx::VertexAttributeArray&& value) override {
        vertexAttributes = std::move(static_cast<VertexAttributeArrayGL&&>(value));
    }

    void setVertexArray(gl::VertexArray&&, gfx::UniqueVertexBufferResource&&, gfx::IndexBuffer&&);

protected:
    std::vector<std::uint8_t> vertData;

    class Impl;
    const std::unique_ptr<Impl> impl;

    // For testing only.
    DrawableGL(std::unique_ptr<Impl>);
    
private:
    VertexAttributeArrayGL vertexAttributes;
};

} // namespace gl
} // namespace mbgl
