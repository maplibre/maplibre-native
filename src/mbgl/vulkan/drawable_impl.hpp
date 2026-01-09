#pragma once

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/uniform.hpp>
#include <mbgl/vulkan/uniform_buffer.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/upload_pass.hpp>
#include <mbgl/vulkan/pipeline.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/util/mat4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mbgl {
namespace vulkan {

using namespace platform;

class Drawable::Impl final {
public:
    Impl()
        : uniformBuffers(
              DescriptorSetType::DrawableUniform, shaders::globalUBOCount, 0, shaders::maxUBOCountPerDrawable) {}

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
} // namespace mbgl
