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

class RenderPass;
class Texture2D;
class VertexArray;

class Drawable : public gfx::Drawable {
public:
    Drawable(std::string name);
    ~Drawable() override;

    void draw(PaintParameters&) const override;

    struct DrawSegment;
    void setIndexData(gfx::IndexVectorBasePtr, std::vector<UniqueDrawSegment> segments) override;

    void setVertices(std::vector<uint8_t>&&, std::size_t, gfx::AttributeDataType) override;

    const gfx::VertexAttributeArray& getVertexAttributes() const override;
    void setVertexAttributes(const gfx::VertexAttributeArray& value) override;
    void setVertexAttributes(gfx::VertexAttributeArray&& value) override;

    gfx::VertexAttributeArray& mutableVertexAttributes() override;

    const gfx::UniformBufferArray& getUniformBuffers() const override;
    gfx::UniformBufferArray& mutableUniformBuffers() override;

    void setVertexAttrNameId(const StringIdentity);

    void upload(gfx::UploadPass&);

    void setColorMode(const gfx::ColorMode&) override;

    void setShader(gfx::ShaderProgramBasePtr) override;

    void setEnableStencil(bool) override;
    void setEnableDepth(bool) override;
    void setSubLayerIndex(int32_t) override;
    void setDepthType(gfx::DepthMaskType) override;

protected:
    // For testing only.
    Drawable(std::unique_ptr<Impl>);

    void bindAttributes(const RenderPass& renderPass) const;
    void unbindAttributes(const RenderPass& renderPass) const;

    void bindUniformBuffers(const RenderPass& renderPass) const;
    void unbindUniformBuffers(const RenderPass& renderPass) const;

    void bindTextures(const RenderPass& renderPass) const;
    void unbindTextures(const RenderPass& renderPass) const;

    void uploadTextures() const;

    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace mtl
} // namespace mbgl
