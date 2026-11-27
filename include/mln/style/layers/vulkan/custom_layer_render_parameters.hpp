#pragma once

#include <mln/style/layers/custom_layer_render_parameters.hpp>
#include <vulkan/vulkan.hpp>

namespace mln {

class PaintParameters;

namespace style {
namespace vulkan {

struct CustomLayerRenderParameters : mln::style::CustomLayerRenderParameters {
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

    /// The terrain depth pass's packed-RGBA output for this frame, or null handles when terrain is off or
    /// the pass has not drawn yet. Pixel size is the drawable's. Each pixel holds the mesh's window depth in
    /// [0, 1] packed as depth = r / 256^3 + g / 256^2 + b / 256 + a (decode with the engine's `unpack_depth`);
    /// uncovered pixels are white. Sample it with the sampler given, which is nearest filtering.
    /// The image is in `terrainDepthImageLayout` (shader read only) while the main render pass records.
    vk::ImageView terrainDepthImageView;
    vk::Sampler terrainDepthSampler;
    vk::ImageLayout terrainDepthImageLayout = vk::ImageLayout::eUndefined;

    explicit CustomLayerRenderParameters(const PaintParameters&);
};

} // namespace vulkan
} // namespace style
} // namespace mln
