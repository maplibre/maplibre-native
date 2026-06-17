#pragma once

#include <mbgl/style/layers/custom_layer_render_parameters.hpp>
#include <vulkan/vulkan.hpp>

namespace mbgl {

class PaintParameters;

namespace style {
namespace vulkan {

struct CustomLayerRenderParameters : mbgl::style::CustomLayerRenderParameters {
    /// Dynamic dispatcher.
    const vk::detail::DispatchLoaderDynamic& dispatcher;

    /// Vulkan device
    vk::Device device;

    /// Compatible render pass handle — use when creating VkPipeline.
    vk::RenderPass renderPass;

    /// Active command buffer for the current render pass — record all draw calls here.
    vk::CommandBuffer commandBuffer;

    /// @brief Screen pre-rotation related to VkSurfaceTransformFlagBitsKHR.
    /// This field is going to be non 0.0 when renderer decides to do the rotation inside the rendering pipeline instead
    /// of relying on compositor. Custom layers must use this parameter to do the rotation in clip space.
    ///
    /// @code{.cpp}
    /// if(screenPreRotationRadiansClockwise != 0.0)
    /// {
    ///     glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
    ///     glm::mat4 pre_rotate_mat = glm::rotate(screenPreRotationRadiansClockwise, rotation_axis);
    ///     MVP = pre_rotate_mat * MVP;
    /// }
    /// @endcode
    ///
    /// @note For more info see https://developer.android.com/games/optimize/vulkan-prerotation.
    /// @attention dFdx, dFdy in shaders also need adjustments if used.
    float screenPreRotationRadiansClockwise = 0.0f;

    explicit CustomLayerRenderParameters(const PaintParameters&);
};

} // namespace vulkan
} // namespace style
} // namespace mbgl
