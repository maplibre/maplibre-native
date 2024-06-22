#pragma once

#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/gfx/color_mode.hpp>

namespace mbgl {
namespace vulkan {

class PipelineInfo final {

public:
    PipelineInfo() noexcept = default;
    ~PipelineInfo() noexcept = default;

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

    bool wideLines = false;

    // dynamic values (not part of the pipeline/ignored in hash)
    struct {
        std::optional<std::array<float, 4>> blendConstants;
        std::optional<float> lineWidth;
    } dynamicValues;

public:

    static vk::Format vulkanFormat(const gfx::AttributeDataType& value);
    static vk::PrimitiveTopology vulkanPrimitiveTopology(const gfx::DrawModeType& value);
    static vk::CullModeFlagBits vulkanCullMode(const gfx::CullFaceSideType& value);
    static vk::FrontFace vulkanFrontFace(const gfx::CullFaceWindingType& value);
    static vk::BlendOp vulkanBlendOp(const gfx::ColorBlendEquationType& value);
    static vk::BlendFactor vulkanBlendFactor(const gfx::ColorBlendFactorType& value);
    static vk::CompareOp vulkanCompareOp(const gfx::DepthFunctionType& value);

    void setCullMode(const gfx::CullFaceMode& value);
    void setTopology(const gfx::DrawModeType& value);
    void setColorBlend(const gfx::ColorMode& value);
    void setColorBlendFunction(const gfx::ColorMode::BlendFunction& value);
    void setDepthWrite(const gfx::DepthMaskType& value);

    bool usesBlendConstants() const;
    std::size_t hash() const;

protected:

};

} // namespace vulkan
} // namespace mbgl
