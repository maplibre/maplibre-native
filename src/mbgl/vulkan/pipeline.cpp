#include <mbgl/vulkan/pipeline.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>

namespace mbgl {
namespace vulkan {

vk::Format PipelineInfo::vulkanFormat(const gfx::AttributeDataType& value) {
    switch (value) {
        case gfx::AttributeDataType::Byte:
            return vk::Format::eR8Sint;
        case gfx::AttributeDataType::Byte2:
            return vk::Format::eR8G8Sint;
        case gfx::AttributeDataType::Byte3:
            return vk::Format::eR8G8B8Sint;
        case gfx::AttributeDataType::Byte4:
            return vk::Format::eR8G8B8A8Sint;
        case gfx::AttributeDataType::UByte:
            return vk::Format::eR8Uint;
        case gfx::AttributeDataType::UByte2:
            return vk::Format::eR8G8Uint;
        case gfx::AttributeDataType::UByte3:
            return vk::Format::eR8G8B8Uint;
        case gfx::AttributeDataType::UByte4:
            return vk::Format::eR8G8B8A8Uint;
        case gfx::AttributeDataType::Short:
            return vk::Format::eR16Sint;
        case gfx::AttributeDataType::Short2:
            return vk::Format::eR16G16Sint;
        case gfx::AttributeDataType::Short3:
            return vk::Format::eR16G16B16Sint;
        case gfx::AttributeDataType::Short4:
            return vk::Format::eR16G16B16A16Sint;
        case gfx::AttributeDataType::UShort:
            return vk::Format::eR16Uint;
        case gfx::AttributeDataType::UShort2:
            return vk::Format::eR16G16Uint;
        case gfx::AttributeDataType::UShort3:
            return vk::Format::eR16G16B16Uint;
        case gfx::AttributeDataType::UShort4:
            return vk::Format::eR16G16B16A16Uint;
        case gfx::AttributeDataType::Int:
            return vk::Format::eR32Sint;
        case gfx::AttributeDataType::Int2:
            return vk::Format::eR32G32Sint;
        case gfx::AttributeDataType::Int3:
            return vk::Format::eR32G32B32Sint;
        case gfx::AttributeDataType::Int4:
            return vk::Format::eR32G32B32A32Sint;
        case gfx::AttributeDataType::UInt:
            return vk::Format::eR32Uint;
        case gfx::AttributeDataType::UInt2:
            return vk::Format::eR32G32Uint;
        case gfx::AttributeDataType::UInt3:
            return vk::Format::eR32G32B32Uint;
        case gfx::AttributeDataType::UInt4:
            return vk::Format::eR32G32B32A32Uint;
        case gfx::AttributeDataType::Float:
            return vk::Format::eR32Sfloat;
        case gfx::AttributeDataType::Float2:
            return vk::Format::eR32G32Sfloat;
        case gfx::AttributeDataType::Float3:
            return vk::Format::eR32G32B32Sfloat;
        case gfx::AttributeDataType::Float4:
            return vk::Format::eR32G32B32A32Sfloat;

        default:
            [[fallthrough]];
        case gfx::AttributeDataType::UShort8:
            [[fallthrough]];
        case gfx::AttributeDataType::Invalid:
            return vk ::Format::eUndefined;
    }
}

vk::PrimitiveTopology PipelineInfo::vulkanPrimitiveTopology(const gfx::DrawModeType& value) {
    switch (value) {
        case gfx::DrawModeType::Points:
            return vk::PrimitiveTopology::ePointList;
        case gfx::DrawModeType::Lines:
            return vk::PrimitiveTopology::eLineList;
        case gfx::DrawModeType::LineLoop:
            return vk::PrimitiveTopology::eLineStrip;
        case gfx::DrawModeType::LineStrip:
            return vk::PrimitiveTopology::eLineStrip;
        case gfx::DrawModeType::Triangles:
            return vk::PrimitiveTopology::eTriangleList;
        case gfx::DrawModeType::TriangleStrip:
            return vk::PrimitiveTopology::eTriangleStrip;
        case gfx::DrawModeType::TriangleFan:
            return vk::PrimitiveTopology::eTriangleFan;
        default:
            return vk::PrimitiveTopology::eTriangleList;
    }
}

vk::CullModeFlagBits PipelineInfo::vulkanCullMode(const gfx::CullFaceSideType& value) {
    switch (value) {
        case gfx::CullFaceSideType::Back:
            return vk::CullModeFlagBits::eBack;
        case gfx::CullFaceSideType::Front:
            return vk::CullModeFlagBits::eFront;
        case gfx::CullFaceSideType::FrontAndBack:
            return vk::CullModeFlagBits::eFrontAndBack;
        default:
            return vk::CullModeFlagBits::eNone;
    }
}

vk::FrontFace PipelineInfo::vulkanFrontFace(const gfx::CullFaceWindingType& value) {
    switch (value) {
        default:
            [[fallthrough]];
        case gfx::CullFaceWindingType::Clockwise:
            return vk::FrontFace::eClockwise;
        case gfx::CullFaceWindingType::CounterClockwise:
            return vk::FrontFace::eCounterClockwise;
    }
}

vk::BlendOp PipelineInfo::vulkanBlendOp(const gfx::ColorBlendEquationType& value) {
    switch (value) {
        default:
            [[fallthrough]];
        case gfx::ColorBlendEquationType::Add:
            return vk::BlendOp::eAdd;
        case gfx::ColorBlendEquationType::Subtract:
            return vk::BlendOp::eSubtract;
        case gfx::ColorBlendEquationType::ReverseSubtract:
            return vk::BlendOp::eReverseSubtract;
    }
}

vk::BlendFactor PipelineInfo::vulkanBlendFactor(const gfx::ColorBlendFactorType& value) {
    switch (value) {
        default:
            [[fallthrough]];
        case gfx::ColorBlendFactorType::Zero:
            return vk::BlendFactor::eZero;
        case gfx::ColorBlendFactorType::One:
            return vk::BlendFactor::eOne;
        case gfx::ColorBlendFactorType::SrcColor:
            return vk::BlendFactor::eSrcColor;
        case gfx::ColorBlendFactorType::OneMinusSrcColor:
            return vk::BlendFactor::eOneMinusSrcColor;
        case gfx::ColorBlendFactorType::SrcAlpha:
            return vk::BlendFactor::eSrcAlpha;
        case gfx::ColorBlendFactorType::OneMinusSrcAlpha:
            return vk::BlendFactor::eOneMinusSrcAlpha;
        case gfx::ColorBlendFactorType::DstAlpha:
            return vk::BlendFactor::eDstAlpha;
        case gfx::ColorBlendFactorType::OneMinusDstAlpha:
            return vk::BlendFactor::eOneMinusDstAlpha;
        case gfx::ColorBlendFactorType::DstColor:
            return vk::BlendFactor::eDstColor;
        case gfx::ColorBlendFactorType::OneMinusDstColor:
            return vk::BlendFactor::eOneMinusDstColor;
        case gfx::ColorBlendFactorType::SrcAlphaSaturate:
            return vk::BlendFactor::eSrcAlphaSaturate;
        case gfx::ColorBlendFactorType::ConstantColor:
            return vk::BlendFactor::eConstantColor;
        case gfx::ColorBlendFactorType::OneMinusConstantColor:
            return vk::BlendFactor::eOneMinusConstantColor;
        case gfx::ColorBlendFactorType::ConstantAlpha:
            return vk::BlendFactor::eConstantAlpha;
        case gfx::ColorBlendFactorType::OneMinusConstantAlpha:
            return vk::BlendFactor::eOneMinusConstantAlpha;
    }
}

vk::CompareOp PipelineInfo::vulkanCompareOp(const gfx::DepthFunctionType& value) {
    switch (value) {
        default:
            [[fallthrough]];
        case gfx::DepthFunctionType::Never:
            return vk::CompareOp::eNever;
        case gfx::DepthFunctionType::Less:
            return vk::CompareOp::eLess;
        case gfx::DepthFunctionType::Equal:
            return vk::CompareOp::eEqual;
        case gfx::DepthFunctionType::LessEqual:
            return vk::CompareOp::eLessOrEqual;
        case gfx::DepthFunctionType::Greater:
            return vk::CompareOp::eGreater;
        case gfx::DepthFunctionType::NotEqual:
            return vk::CompareOp::eNotEqual;
        case gfx::DepthFunctionType::GreaterEqual:
            return vk::CompareOp::eGreaterOrEqual;
        case gfx::DepthFunctionType::Always:
            return vk::CompareOp::eAlways;
    }
}

vk::CompareOp PipelineInfo::vulkanCompareOp(const gfx::StencilFunctionType& value) {
    switch (value) {
        default:
            [[fallthrough]];
        case gfx::StencilFunctionType::Never:
            return vk::CompareOp::eNever;
        case gfx::StencilFunctionType::Less:
            return vk::CompareOp::eLess;
        case gfx::StencilFunctionType::Equal:
            return vk::CompareOp::eEqual;
        case gfx::StencilFunctionType::LessEqual:
            return vk::CompareOp::eLessOrEqual;
        case gfx::StencilFunctionType::Greater:
            return vk::CompareOp::eGreater;
        case gfx::StencilFunctionType::NotEqual:
            return vk::CompareOp::eNotEqual;
        case gfx::StencilFunctionType::GreaterEqual:
            return vk::CompareOp::eGreaterOrEqual;
        case gfx::StencilFunctionType::Always:
            return vk::CompareOp::eAlways;
    }
}

vk::StencilOp PipelineInfo::vulkanStencilOp(const gfx::StencilOpType& value) {
    switch (value) {
        default:
            [[fallthrough]];
        case gfx::StencilOpType::Zero:
            return vk::StencilOp::eZero;
        case gfx::StencilOpType::Keep:
            return vk::StencilOp::eKeep;
        case gfx::StencilOpType::Replace:
            return vk::StencilOp::eReplace;
        case gfx::StencilOpType::Increment:
            return vk::StencilOp::eIncrementAndClamp;
        case gfx::StencilOpType::Decrement:
            return vk::StencilOp::eDecrementAndClamp;
        case gfx::StencilOpType::Invert:
            return vk::StencilOp::eInvert;
        case gfx::StencilOpType::IncrementWrap:
            return vk::StencilOp::eIncrementAndWrap;
        case gfx::StencilOpType::DecrementWrap:
            return vk::StencilOp::eDecrementAndWrap;
    }
}

void PipelineInfo::setCullMode(const gfx::CullFaceMode& value) {
    cullMode = value.enabled ? vulkanCullMode(value.side) : vk::CullModeFlagBits::eNone;
    frontFace = vulkanFrontFace(value.winding);
}

void PipelineInfo::setDrawMode(const gfx::DrawModeType& value) {
    topology = vulkanPrimitiveTopology(value);
}

void PipelineInfo::setDrawMode(const gfx::DrawMode& value) {
    topology = vulkanPrimitiveTopology(value.type);

    if (value.type == gfx::DrawModeType::Lines || value.type == gfx::DrawModeType::LineStrip ||
        value.type == gfx::DrawModeType::LineLoop) {
        setLineWidth(value.size);
    } else {
        setLineWidth(1.0f);
    }
}

void PipelineInfo::setColorBlend(const gfx::ColorMode& value) {
    setColorBlendFunction(value.blendFunction);

    if (usesBlendConstants()) {
        dynamicValues.blendConstants = value.blendColor;
    } else {
        dynamicValues.blendConstants.reset();
    }

    colorMask = vk::ColorComponentFlags();

    if (value.mask.r) colorMask |= vk::ColorComponentFlagBits::eR;
    if (value.mask.g) colorMask |= vk::ColorComponentFlagBits::eG;
    if (value.mask.b) colorMask |= vk::ColorComponentFlagBits::eB;
    if (value.mask.a) colorMask |= vk::ColorComponentFlagBits::eA;
}

void PipelineInfo::setColorBlendFunction(const gfx::ColorMode::BlendFunction& value) {
    colorBlend = !value.is<gfx::ColorMode::Replace>();

    apply_visitor(
        [&](const auto& blendFunction) {
            colorBlendFunction = vulkanBlendOp(blendFunction.equation);
            srcBlendFactor = vulkanBlendFactor(blendFunction.srcFactor);
            dstBlendFactor = vulkanBlendFactor(blendFunction.dstFactor);
        },
        value);
}

void PipelineInfo::setDepthWrite(const gfx::DepthMaskType& value) {
    switch (value) {
        default:
            [[fallthrough]];
        case gfx::DepthMaskType::ReadOnly:
            depthWrite = false;
            break;
        case gfx::DepthMaskType::ReadWrite:
            depthWrite = true;
            break;
    }
}

void PipelineInfo::setDepthMode(const gfx::DepthMode& value) {
    depthFunction = vulkanCompareOp(value.func);
    setDepthWrite(value.mask);
}

void PipelineInfo::setStencilMode(const gfx::StencilMode& value) {
    if (value.test.is<gfx::StencilMode::Always>()) {
        stencilTest = false;
        stencilFunction = vk::CompareOp::eNever;
        return;
    }

    stencilTest = true;

    if (value.test.is<gfx::StencilMode::Never>()) {
        stencilFunction = vk::CompareOp::eNever;
        return;
    }

    apply_visitor(
        [&](const auto& test) {
            stencilFunction = vulkanCompareOp(test.func);
            dynamicValues.stencilCompareMask = test.mask;
        },
        value.test);

    dynamicValues.stencilWriteMask = value.mask;
    dynamicValues.stencilRef = value.ref;

    stencilPass = vulkanStencilOp(value.pass);
    stencilFail = vulkanStencilOp(value.fail);
    stencilDepthFail = vulkanStencilOp(value.depthFail);
}

void PipelineInfo::setRenderable(const gfx::Renderable& value) {
    const auto& renderableResource = value.getResource<RenderableResource>();

    renderPass = renderableResource.getRenderPass().get();
    viewExtent = renderableResource.getExtent();
}

void PipelineInfo::setLineWidth(float value) {
    wideLines = value != 1.0f;
    dynamicValues.lineWidth = value;
}

bool PipelineInfo::usesBlendConstants() const {
    if (srcBlendFactor == vk::BlendFactor::eConstantAlpha || srcBlendFactor == vk::BlendFactor::eConstantColor ||
        srcBlendFactor == vk::BlendFactor::eOneMinusConstantAlpha ||
        srcBlendFactor == vk::BlendFactor::eOneMinusConstantColor) {
        return true;
    }

    if (dstBlendFactor == vk::BlendFactor::eConstantAlpha || dstBlendFactor == vk::BlendFactor::eConstantColor ||
        dstBlendFactor == vk::BlendFactor::eOneMinusConstantAlpha ||
        dstBlendFactor == vk::BlendFactor::eOneMinusConstantColor) {
        return true;
    }

    return false;
}

void PipelineInfo::updateVertexInputHash() {
    vertexInputHash = 0;

    for (const auto& value : inputAttributes) {
        util::hash_combine(vertexInputHash, value.binding);
        util::hash_combine(vertexInputHash, value.format);
        util::hash_combine(vertexInputHash, value.location);
        util::hash_combine(vertexInputHash, value.offset);
    }

    for (const auto& value : inputBindings) {
        util::hash_combine(vertexInputHash, value.binding);
        util::hash_combine(vertexInputHash, value.inputRate);
    }
}

std::size_t PipelineInfo::hash() const {
    return util::hash(topology,
                      cullMode,
                      frontFace,
                      polygonMode,
                      colorBlend,
                      colorBlendFunction,
                      srcBlendFactor,
                      dstBlendFactor,
                      VkColorComponentFlags(colorMask),
                      depthTest,
                      depthWrite,
                      depthFunction,
                      stencilTest,
                      stencilFunction,
                      stencilPass,
                      stencilFail,
                      stencilDepthFail,
                      wideLines,
                      VkRenderPass(renderPass),
                      vertexInputHash);
}

void PipelineInfo::setDynamicValues(const RendererBackend& backend, const vk::UniqueCommandBuffer& buffer) const {
    if (dynamicValues.blendConstants.has_value()) {
        buffer->setBlendConstants(dynamicValues.blendConstants.value().data());
    }

    if (stencilTest) {
        buffer->setStencilWriteMask(vk::StencilFaceFlagBits::eFrontAndBack, dynamicValues.stencilWriteMask);
        buffer->setStencilCompareMask(vk::StencilFaceFlagBits::eFrontAndBack, dynamicValues.stencilCompareMask);
        buffer->setStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, dynamicValues.stencilRef);
    }

    if (backend.getDeviceFeatures().wideLines && wideLines) {
        buffer->setLineWidth(dynamicValues.lineWidth);
    }
}

std::vector<vk::DynamicState> PipelineInfo::getDynamicStates(const RendererBackend& backend) const {
    std::vector<vk::DynamicState> dynamicStates;

    if (usesBlendConstants()) {
        dynamicStates.push_back(vk::DynamicState::eBlendConstants);
    }

    if (stencilTest) {
        dynamicStates.push_back(vk::DynamicState::eStencilCompareMask);
        dynamicStates.push_back(vk::DynamicState::eStencilWriteMask);
        dynamicStates.push_back(vk::DynamicState::eStencilReference);
    }

    if (backend.getDeviceFeatures().wideLines && wideLines) {
        dynamicStates.push_back(vk::DynamicState::eLineWidth);
    }

    return dynamicStates;
}

} // namespace vulkan
} // namespace mbgl
