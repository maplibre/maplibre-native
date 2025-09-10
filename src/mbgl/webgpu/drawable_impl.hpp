#pragma once

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/vertex_attribute.hpp>
#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/mat4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mbgl {
namespace webgpu {

class Drawable::Impl final {
public:
    Impl() = default;
    ~Impl() = default;

    std::vector<UniqueDrawSegment> segments;

    WGPURenderPipeline pipeline = nullptr;
    WGPUBindGroup bindGroup = nullptr;
    
    gfx::IndexVectorBasePtr indexes;
    std::size_t vertexCount = 0;
    gfx::AttributeDataType vertexType = gfx::AttributeDataType::Invalid;

    gfx::AttributeBindingArray attributeBindings;
    gfx::AttributeBindingArray instanceBindings;

    UniformBufferArray uniformBuffers;

    gfx::DepthMode depthMode = gfx::DepthMode::disabled();
    gfx::StencilMode stencilMode;
    gfx::CullFaceMode cullFaceMode;
    std::size_t vertexAttrId = 0;

    std::optional<gfx::RenderPassDescriptor> renderPassDescriptor;
};

// WebGPU uses the base DrawSegment implementation directly
using DrawSegment = gfx::Drawable::DrawSegment;

} // namespace webgpu
} // namespace mbgl