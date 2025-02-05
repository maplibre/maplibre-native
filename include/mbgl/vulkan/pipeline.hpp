#pragma once

#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>

namespace mbgl {
namespace vulkan {

class PipelineInfo final {
public:
    PipelineInfo() noexcept = default;
    ~PipelineInfo() noexcept = default;

    // used to select the pipeline layout
    bool usePushConstants = false;

    vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
    vk::CullModeFlagBits cullMode = vk::CullModeFlagBits::eNone;
    vk::FrontFace frontFace = vk::FrontFace::eClockwise;
    vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

    bool colorBlend = true;
    vk::BlendOp colorBlendFunction = vk::BlendOp::eAdd;
    vk::BlendFactor srcBlendFactor = vk::BlendFactor::eOne;
    vk::BlendFactor dstBlendFactor = vk::BlendFactor::eZero;
    vk::ColorComponentFlags colorMask = vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags;

    bool depthTest = false;
    bool depthWrite = true;
    vk::CompareOp depthFunction = vk::CompareOp::eAlways;

    bool stencilTest = false;
    vk::CompareOp stencilFunction = vk::CompareOp::eNever;
    vk::StencilOp stencilPass = vk::StencilOp::eKeep;
    vk::StencilOp stencilFail = vk::StencilOp::eKeep;
    vk::StencilOp stencilDepthFail = vk::StencilOp::eKeep;

    bool wideLines = false;

    // external values (used in hash)
    vk::RenderPass renderPass{};
    vk::Extent2D viewExtent{};

    // dynamic values (not part of the pipeline/ignored in hash)
    struct {
        std::optional<std::array<float, 4>> blendConstants;
        uint32_t stencilWriteMask = 0;
        uint32_t stencilCompareMask = 0;
        uint32_t stencilRef = 0;
        float lineWidth = 1.0f;
    } dynamicValues;

    std::vector<vk::VertexInputBindingDescription> inputBindings;
    std::vector<vk::VertexInputAttributeDescription> inputAttributes;

public:
    static vk::Format vulkanFormat(const gfx::AttributeDataType& value);
    static vk::PrimitiveTopology vulkanPrimitiveTopology(const gfx::DrawModeType& value);
    static vk::CullModeFlagBits vulkanCullMode(const gfx::CullFaceSideType& value);
    static vk::FrontFace vulkanFrontFace(const gfx::CullFaceWindingType& value);
    static vk::BlendOp vulkanBlendOp(const gfx::ColorBlendEquationType& value);
    static vk::BlendFactor vulkanBlendFactor(const gfx::ColorBlendFactorType& value);
    static vk::CompareOp vulkanCompareOp(const gfx::DepthFunctionType& value);
    static vk::CompareOp vulkanCompareOp(const gfx::StencilFunctionType& value);
    static vk::StencilOp vulkanStencilOp(const gfx::StencilOpType& value);

    void setCullMode(const gfx::CullFaceMode& value);
    void setDrawMode(const gfx::DrawModeType& value);
    void setDrawMode(const gfx::DrawMode& value);
    void setColorBlend(const gfx::ColorMode& value);
    void setColorBlendFunction(const gfx::ColorMode::BlendFunction& value);
    void setDepthWrite(const gfx::DepthMaskType& value);
    void setDepthMode(const gfx::DepthMode& value);
    void setStencilMode(const gfx::StencilMode& value);
    void setRenderable(const gfx::Renderable& value);
    void setLineWidth(float value);

    bool usesBlendConstants() const;
    void updateVertexInputHash();
    std::size_t hash() const;

    void setDynamicValues(const RendererBackend& backend, const vk::UniqueCommandBuffer& buffer) const;
    std::vector<vk::DynamicState> getDynamicStates(const RendererBackend& backend) const;

protected:
    std::size_t vertexInputHash{0};
};

} // namespace vulkan
} // namespace mbgl
