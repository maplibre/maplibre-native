#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/vulkan/upload_pass.hpp>
#include <mbgl/programs/segment.hpp>

#include <memory>

namespace mbgl {

template <class AttributeList>
class Segment;
class PaintParameters;

namespace gfx {

class IndexBuffer;
class UploadPass;
class VertexBufferResource;

using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;

} // namespace gfx

namespace vulkan {

class RenderPass;
class Texture2D;
class VertexArray;

class Drawable : public gfx::Drawable {
public:
    Drawable(std::string name);
    ~Drawable() override;

    void upload(gfx::UploadPass&);
    void draw(PaintParameters&) const override;

    void setIndexData(gfx::IndexVectorBasePtr, std::vector<UniqueDrawSegment> segments) override;
    void setVertices(std::vector<uint8_t>&&, std::size_t, gfx::AttributeDataType) override;

    const gfx::UniformBufferArray& getUniformBuffers() const override;
    gfx::UniformBufferArray& mutableUniformBuffers() override;

    void setEnableColor(bool value) override;
    void setColorMode(const gfx::ColorMode& value) override;
    void setEnableDepth(bool value) override;
    void setDepthType(gfx::DepthMaskType value) override;
    void setEnableStencil(bool value) override;

    void setLineWidth(int32_t value) override;
    void setCullFaceMode(const gfx::CullFaceMode&) override;

protected:
    void buildVulkanInputBindings() noexcept;

    bool bindAttributes(CommandEncoder&) const noexcept;
    bool bindDescriptors(CommandEncoder&) const noexcept;

    void uploadTextures(UploadPass&) const noexcept;

    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace vulkan
} // namespace mbgl
