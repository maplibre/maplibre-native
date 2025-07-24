#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/draw_mode.hpp>

#include <mbgl/gfx/scissor_rect.hpp>
#include <memory>

namespace mbgl {

namespace gfx {

class UploadPass;
class DepthMode;
class StencilMode;

} // namespace gfx

namespace vulkan {

class CommandEncoder;
class UploadPass;

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
    void setDepthModeFor3D(const gfx::DepthMode& value);
    void setStencilModeFor3D(const gfx::StencilMode& value);

    void setLineWidth(int32_t value) override;
    void setCullFaceMode(const gfx::CullFaceMode&) override;
    void setScissorRect(const gfx::ScissorRect&) override;

    void updateVertexAttributes(gfx::VertexAttributeArrayPtr,
                                std::size_t vertexCount,
                                gfx::DrawMode,
                                gfx::IndexVectorBasePtr,
                                const SegmentBase* segments,
                                std::size_t segmentCount) override;

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
