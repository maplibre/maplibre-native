#pragma once

#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>

namespace mbgl {
namespace gl {

/**
    Base class for OpenGL-specific drawable builders.
 */
class DrawableGLBuilder final : public gfx::DrawableBuilder {
public:
    DrawableGLBuilder() {
    }
    virtual ~DrawableGLBuilder() = default;
    
    const gfx::VertexAttributeArray& getVertexAttributes() const override { return vertexAttributes; }
    void setVertexAttributes(const VertexAttributeArrayGL& value) { vertexAttributes = value; }
    void setVertexAttributes(VertexAttributeArrayGL&& value) { vertexAttributes = std::move(value); }
    
protected:
    gfx::DrawablePtr createDrawable() const override;
    
    /// Setup the SDK-specific aspects after all the values are present
    void init() override;
    
private:
    VertexAttributeArrayGL vertexAttributes;
};

} // namespace gl
} // namespace mbgl
