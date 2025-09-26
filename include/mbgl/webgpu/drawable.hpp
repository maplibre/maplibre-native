#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <webgpu/webgpu.h>
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
    Drawable(std::string name);
    ~Drawable() override;

    void upload(gfx::UploadPass&);
    void draw(PaintParameters&) const override;

    void setIndexData(gfx::IndexVectorBasePtr, std::vector<UniqueDrawSegment> segments) override;
    void setVertices(std::vector<uint8_t>&&, std::size_t, gfx::AttributeDataType) override;

    const gfx::UniformBufferArray& getUniformBuffers() const override;
    gfx::UniformBufferArray& mutableUniformBuffers() override;

    void setColorMode(const gfx::ColorMode&) override;

    void setShader(gfx::ShaderProgramBasePtr) override;

    void setEnableStencil(bool) override;
    void setEnableDepth(bool) override;
    void setSubLayerIndex(int32_t) override;
    void setDepthType(gfx::DepthMaskType) override;
    void setDepthModeFor3D(const gfx::DepthMode&);
    void setStencilModeFor3D(const gfx::StencilMode&);

    void setVertexAttrId(const size_t);

    void updateVertexAttributes(gfx::VertexAttributeArrayPtr,
                                std::size_t vertexCount,
                                gfx::DrawMode,
                                gfx::IndexVectorBasePtr,
                                const SegmentBase* segments,
                                std::size_t segmentCount) override;

    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace webgpu
} // namespace mbgl
