#pragma once

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/program.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/vulkan/uniform_buffer.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/upload_pass.hpp>
#include <mbgl/vulkan/pipeline.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/mat4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mbgl {
namespace vulkan {

using namespace platform;

class Drawable::Impl final {
public:
    Impl() = default;
    ~Impl() = default;

    std::vector<UniqueDrawSegment> segments;

    gfx::IndexVectorBasePtr indexes;
    std::size_t vertexCount = 0;
    gfx::AttributeDataType vertexType = gfx::AttributeDataType::Invalid;

    std::vector<gfx::UniqueVertexBufferResource> attributeBuffers;
    gfx::AttributeBindingArray attributeBindings;

    std::vector<gfx::UniqueVertexBufferResource> instanceBuffers;
    gfx::AttributeBindingArray instanceBindings;

    vulkan::UniformBufferArray uniformBuffers;

    std::size_t vertexAttrId = 0;

    std::optional<gfx::RenderPassDescriptor> renderPassDescriptor;

    PipelineInfo pipelineInfo;
};

} // namespace vulkan
} // namespace mbgl
