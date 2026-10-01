#pragma once

#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/drawable_impl.hpp>
#include <mln/gfx/index_buffer.hpp>
#include <mln/gfx/uniform.hpp>
#include <mln/gfx/uniform_buffer.hpp>
#include <mln/webgpu/uniform_buffer.hpp>
#include <mln/webgpu/render_pass.hpp>
#include <mln/webgpu/upload_pass.hpp>
#include <mln/webgpu/vertex_attribute.hpp>
#include <mln/webgpu/vertex_buffer_resource.hpp>
#include <webgpu/webgpu.h>
#include <mln/shaders/segment.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/util/mat4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mln {
namespace webgpu {

class Drawable::Impl final {
public:
    Impl() = default;
    ~Impl() = default;

    std::vector<UniqueDrawSegment> segments;

    // Vertex description (similar to Metal's vertexDesc/vertexDescHash)
    std::size_t vertexDescHash = 0;

    // Index data (matching Metal's indexes)
    gfx::IndexVectorBasePtr indexes;
    std::size_t vertexCount = 0;
    gfx::AttributeDataType vertexType = gfx::AttributeDataType::Invalid;

    gfx::AttributeBindingArray attributeBindings;
    gfx::AttributeBindingArray instanceBindings;

    // Store dummy vertex buffers to keep them alive
    std::vector<std::unique_ptr<gfx::VertexBufferResource>> dummyVertexBuffers;

    UniformBufferArray uniformBuffers;
    gfx::UniformBufferPtr uboIndexUniform;

    // Render state (matching Metal)
    gfx::DepthMode depthMode = gfx::DepthMode::disabled();
    gfx::StencilMode stencilMode;
    gfx::CullFaceMode cullFaceMode;
    std::size_t vertexAttrId = 0;
    std::size_t stencilModeHash = 0;
    bool stencilModeHashValid = false;

    // Pipeline state (WebGPU equivalent of Metal's pipelineState)
    WGPURenderPipeline pipelineState = nullptr;

    std::optional<gfx::RenderPassDescriptor> renderPassDescriptor;

    // WebGPU-specific state (similar to Metal's depthStencilState)
    WGPUDepthStencilState depthStencilState = {};
    gfx::StencilMode previousStencilMode;

    // WebGPU-specific resources needed for rendering
    struct BindGroupRecord {
        uint32_t slot = 0;
        uint32_t group = 0;
        WGPUBindGroup handle = nullptr;
    };
    std::vector<BindGroupRecord> bindGroups;
};

// WebGPU-specific DrawSegment inheriting from the base gfx::Drawable::DrawSegment
using DrawSegment = gfx::Drawable::DrawSegment;

} // namespace webgpu
} // namespace mln
