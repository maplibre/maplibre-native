#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/programs/segment.hpp>

#include <memory>

namespace mbgl {

template <class AttributeList>
class Segment;

namespace gfx {

class IndexBuffer;
class UploadPass;
class VertexBufferResource;

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

} // namespace gfx

namespace mtl {

class Texture2D;
class VertexArray;

class Drawable : public gfx::Drawable {
public:
    Drawable(std::string name);
    ~Drawable() override;

    void draw(PaintParameters&) const override;

    void setIndexData(gfx::IndexVectorBasePtr, std::vector<UniqueDrawSegment> segments) override;

    void setVertices(std::vector<uint8_t>&&, std::size_t, gfx::AttributeDataType) override;

    const gfx::VertexAttributeArray& getVertexAttributes() const override;
    void setVertexAttributes(const gfx::VertexAttributeArray& value) override;
    void setVertexAttributes(gfx::VertexAttributeArray&& value) override;

    gfx::VertexAttributeArray& mutableVertexAttributes() override;

    const gfx::UniformBufferArray& getUniformBuffers() const override;
    gfx::UniformBufferArray& mutableUniformBuffers() override;

    // void setVertexAttrName(std::string);

    // void upload(gfx::UploadPass&);

protected:
    class Impl;
    const std::unique_ptr<Impl> impl;

    // For testing only.
    Drawable(std::unique_ptr<Impl>);

private:
    /*
    gfx::ColorMode makeColorMode(PaintParameters&) const;
    gfx::StencilMode makeStencilMode(PaintParameters&) const;

    void uploadTextures() const;

    void bindUniformBuffers() const;
    void unbindUniformBuffers() const;

    void bindTextures() const;
    void unbindTextures() const;
     */
};

} // namespace mtl
} // namespace mbgl
