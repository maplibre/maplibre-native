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

    const gfx::UniformBufferArray& getUniformBuffers() const override;
    gfx::UniformBufferArray& mutableUniformBuffers() override;

    void setVertexAttrId(const size_t);

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

    void bindAttributes(RenderPass&) const noexcept;
    void unbindAttributes(RenderPass&) const noexcept {}

    void bindUniformBuffers(RenderPass&) const noexcept;
    void unbindUniformBuffers(RenderPass&) const noexcept {}

    void bindTextures(RenderPass&) const noexcept;
    void unbindTextures(RenderPass&) const noexcept;

    void uploadTextures(UploadPass&) const noexcept;

    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace mtl
} // namespace mbgl
