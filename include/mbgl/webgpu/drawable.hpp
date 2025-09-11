#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <memory>
#include <vector>

// Forward declare WebGPU types
typedef struct WGPUBindGroupLayoutImpl* WGPUBindGroupLayout;

namespace mbgl {
namespace gfx {
class UploadPass;
class DepthMode;
class StencilMode;
} // namespace gfx

namespace webgpu {

class CommandEncoder;
class UploadPass;
class RenderPipeline;

class Drawable : public gfx::Drawable {
public:
    explicit Drawable(std::string name);
    ~Drawable() override;

    void upload(gfx::UploadPass&);
    void uploadTextures(UploadPass&) const noexcept;
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
    void setVertexAttrId(std::size_t id);
    
    void updateVertexAttributes(gfx::VertexAttributeArrayPtr,
                               std::size_t vertexCount,
                               gfx::DrawMode,
                               gfx::IndexVectorBasePtr,
                               const SegmentBase* segments,
                               std::size_t segmentCount) override;

protected:
    void buildWebGPUPipeline() noexcept;
    void createBindGroup(WGPUBindGroupLayout layout) noexcept;

    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace webgpu
} // namespace mbgl