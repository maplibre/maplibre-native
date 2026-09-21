#pragma once

#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/drawable_impl.hpp>
#include <mln/gfx/index_buffer.hpp>
#include <mln/gfx/uniform.hpp>
#include <mln/vulkan/uniform_buffer.hpp>
#include <mln/vulkan/render_pass.hpp>
#include <mln/vulkan/upload_pass.hpp>
#include <mln/vulkan/pipeline.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/util/mat4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mln {
namespace vulkan {

using namespace platform;

class Drawable::Impl final {
public:
    Impl()
        : uniformBuffers(DescriptorSetType::DrawableUniform,
                         shaders::drawableSSBOStartId,
                         shaders::maxSSBOCountPerDrawable,
                         shaders::maxUBOCountPerDrawable) {}

    ~Impl() = default;

    std::vector<UniqueDrawSegment> segments;

    gfx::IndexVectorBasePtr indexes;
    std::size_t vertexCount = 0;
    gfx::AttributeDataType vertexType = gfx::AttributeDataType::Invalid;

    gfx::AttributeBindingArray attributeBindings;
    gfx::AttributeBindingArray instanceBindings;

    vulkan::UniformBufferArray uniformBuffers;

    std::size_t vertexAttrId = 0;

    std::optional<gfx::RenderPassDescriptor> renderPassDescriptor;

    std::optional<gfx::DepthMode> depthFor3D;
    std::optional<gfx::StencilMode> stencilFor3D;

    PipelineInfo pipelineInfo;

    std::vector<vk::Buffer> vulkanVertexBuffers;
    std::vector<vk::DeviceSize> vulkanVertexOffsets;

    std::unique_ptr<ImageDescriptorSet> imageDescriptorSet;
};

} // namespace vulkan
} // namespace mln
