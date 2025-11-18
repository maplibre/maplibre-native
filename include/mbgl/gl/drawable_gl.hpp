#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>

#include <memory>

namespace mbgl {

class SegmentBase;
class PaintParameters;

namespace gfx {

class IndexBuffer;
class UploadPass;
class VertexBufferResource;

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

} // namespace gfx

namespace gl {

class Texture2D;
class VertexArray;

class DrawableGL : public gfx::Drawable {
public:
    DrawableGL(std::string name);
    ~DrawableGL() override;

    void draw(PaintParameters&) const override;

    struct DrawSegmentGL;
    void setIndexData(gfx::IndexVectorBasePtr, std::vector<UniqueDrawSegment> segments) override;

    void setVertices(std::vector<uint8_t>&&, std::size_t, gfx::AttributeDataType) override;

    const gfx::UniformBufferArray& getUniformBuffers() const override;
    gfx::UniformBufferArray& mutableUniformBuffers() override;

    void setVertexAttrId(const size_t id);

    void upload(gfx::UploadPass&);

    void updateVertexAttributes(gfx::VertexAttributeArrayPtr,
                                std::size_t vertexCount,
                                gfx::DrawMode,
                                gfx::IndexVectorBasePtr,
                                const SegmentBase* segments,
                                std::size_t segmentCount) override;

protected:
    class Impl;
    const std::unique_ptr<Impl> impl;

    // For testing only.
    DrawableGL(std::unique_ptr<Impl>);

private:
    gfx::ColorMode makeColorMode(PaintParameters&) const;
    gfx::StencilMode makeStencilMode(PaintParameters&) const;

    void uploadTextures() const;

    void bindTextures() const;
    void unbindTextures() const;
};

} // namespace gl
} // namespace mbgl
