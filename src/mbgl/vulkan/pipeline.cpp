#include <mbgl/vulkan/pipeline.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/util/hash.hpp>

namespace mbgl {
namespace vulkan {

vk::Format PipelineInfo::vulkanFormat(const gfx::AttributeDataType& value) {
    switch (value) {
        case gfx::AttributeDataType::Byte:	    return vk::Format::eR8Sscaled;
        case gfx::AttributeDataType::Byte2:	    return vk::Format::eR8G8Sscaled;
        case gfx::AttributeDataType::Byte3:	    return vk::Format::eR8G8B8Sscaled;
        case gfx::AttributeDataType::Byte4:	    return vk::Format::eR8G8B8A8Sscaled;
        case gfx::AttributeDataType::UByte:	    return vk::Format::eR8Uscaled;
        case gfx::AttributeDataType::UByte2:	return vk::Format::eR8G8Uscaled;
        case gfx::AttributeDataType::UByte3:	return vk::Format::eR8G8B8Uscaled;
        case gfx::AttributeDataType::UByte4:	return vk::Format::eR8G8B8A8Uscaled;
        case gfx::AttributeDataType::Short:	    return vk::Format::eR16Sscaled;
        case gfx::AttributeDataType::Short2:	return vk::Format::eR16G16Sscaled;
        case gfx::AttributeDataType::Short3:	return vk::Format::eR16G16B16Sscaled;
        case gfx::AttributeDataType::Short4:	return vk::Format::eR16G16B16A16Sscaled;
        case gfx::AttributeDataType::UShort:	return vk::Format::eR16Uscaled;
        case gfx::AttributeDataType::UShort2:	return vk::Format::eR16G16Uscaled;
        case gfx::AttributeDataType::UShort3:	return vk::Format::eR16G16B16Uscaled;
        case gfx::AttributeDataType::UShort4:	return vk::Format::eR16G16B16A16Uscaled;
        case gfx::AttributeDataType::Int:	    return vk::Format::eR32Sint;
        case gfx::AttributeDataType::Int2:	    return vk::Format::eR32G32Sint;
        case gfx::AttributeDataType::Int3:	    return vk::Format::eR32G32B32Sint;
        case gfx::AttributeDataType::Int4:	    return vk::Format::eR32G32B32A32Sint;
        case gfx::AttributeDataType::UInt:	    return vk::Format::eR32Uint;
        case gfx::AttributeDataType::UInt2:	    return vk::Format::eR32G32Uint;
        case gfx::AttributeDataType::UInt3:	    return vk::Format::eR32G32B32Uint;
        case gfx::AttributeDataType::UInt4:	    return vk::Format::eR32G32B32A32Uint;
        case gfx::AttributeDataType::Float:	    return vk::Format::eR32Sfloat;
        case gfx::AttributeDataType::Float2:	return vk::Format::eR32G32Sfloat;
        case gfx::AttributeDataType::Float3:	return vk::Format::eR32G32B32Sfloat;
        case gfx::AttributeDataType::Float4:	return vk::Format::eR32G32B32A32Sfloat;

        default:                                [[fallthrough]];
        case gfx::AttributeDataType::UShort8:   [[fallthrough]];
        case gfx::AttributeDataType::Invalid:   return vk ::Format::eUndefined;
    }
}

vk::PrimitiveTopology PipelineInfo::vulkanPrimitiveTopology(const gfx::DrawModeType& value) {
     switch (value) {
        case gfx::DrawModeType::Points:         return vk::PrimitiveTopology::ePointList;
        case gfx::DrawModeType::Lines:          return vk::PrimitiveTopology::eLineList;
        case gfx::DrawModeType::LineLoop:       return vk::PrimitiveTopology::eLineStrip;
        case gfx::DrawModeType::LineStrip:      return vk::PrimitiveTopology::eLineStrip;
        case gfx::DrawModeType::Triangles:      return vk::PrimitiveTopology::eTriangleList;
        case gfx::DrawModeType::TriangleStrip:  return vk::PrimitiveTopology::eTriangleStrip;
        case gfx::DrawModeType::TriangleFan:    return vk::PrimitiveTopology::eTriangleFan;
        default:                                return vk::PrimitiveTopology::eTriangleList;
    }
}

vk::CullModeFlagBits PipelineInfo::vulkanCullMode(const gfx::CullFaceSideType& value) {
    switch (value) {
        case gfx::CullFaceSideType::Back:           return vk::CullModeFlagBits::eBack;
        case gfx::CullFaceSideType::Front:          return vk::CullModeFlagBits::eFront;
        case gfx::CullFaceSideType::FrontAndBack:   return vk::CullModeFlagBits::eFrontAndBack;
        default:                                    return vk::CullModeFlagBits::eNone;
    }
}

vk::FrontFace PipelineInfo::vulkanFrontFace(const gfx::CullFaceWindingType& value) {
    switch (value) {
        default:                                            [[fallthrough]];
        case gfx::CullFaceWindingType::Clockwise:           return vk::FrontFace::eClockwise;
        case gfx::CullFaceWindingType::CounterClockwise:    return vk::FrontFace::eCounterClockwise;
    }
}

vk::BlendOp PipelineInfo::vulkanBlendOp(const gfx::ColorBlendEquationType& value) {
    switch (value) {
        default:                                            [[fallthrough]];
        case gfx::ColorBlendEquationType::Add:              return vk::BlendOp::eAdd;
        case gfx::ColorBlendEquationType::Subtract:         return vk::BlendOp::eSubtract;
        case gfx::ColorBlendEquationType::ReverseSubtract:  return vk::BlendOp::eReverseSubtract;
    }
}

vk::BlendFactor PipelineInfo::vulkanBlendFactor(const gfx::ColorBlendFactorType& value) {
    switch (value) {
        default:                                                [[fallthrough]];
        case gfx::ColorBlendFactorType::Zero:                   return vk::BlendFactor::eZero;
        case gfx::ColorBlendFactorType::One:                    return vk::BlendFactor::eOne;
        case gfx::ColorBlendFactorType::SrcColor:               return vk::BlendFactor::eSrcColor;
        case gfx::ColorBlendFactorType::OneMinusSrcColor:       return vk::BlendFactor::eOneMinusSrcColor;
        case gfx::ColorBlendFactorType::SrcAlpha:               return vk::BlendFactor::eSrcAlpha;
        case gfx::ColorBlendFactorType::OneMinusSrcAlpha:       return vk::BlendFactor::eOneMinusSrcAlpha;
        case gfx::ColorBlendFactorType::DstAlpha:               return vk::BlendFactor::eDstAlpha;
        case gfx::ColorBlendFactorType::OneMinusDstAlpha:       return vk::BlendFactor::eOneMinusDstAlpha;
        case gfx::ColorBlendFactorType::DstColor:               return vk::BlendFactor::eDstColor;
        case gfx::ColorBlendFactorType::OneMinusDstColor:       return vk::BlendFactor::eOneMinusDstColor;
        case gfx::ColorBlendFactorType::SrcAlphaSaturate:       return vk::BlendFactor::eSrcAlphaSaturate;
        case gfx::ColorBlendFactorType::ConstantColor:          return vk::BlendFactor::eConstantColor;
        case gfx::ColorBlendFactorType::OneMinusConstantColor:  return vk::BlendFactor::eOneMinusConstantColor;
        case gfx::ColorBlendFactorType::ConstantAlpha:          return vk::BlendFactor::eConstantAlpha;
        case gfx::ColorBlendFactorType::OneMinusConstantAlpha:  return vk::BlendFactor::eOneMinusConstantAlpha;
    }
}

vk::CompareOp PipelineInfo::vulkanCompareOp(const gfx::DepthFunctionType& value) {
    switch (value) {
        default:                                        [[fallthrough]];
        case gfx::DepthFunctionType::Never:             return vk::CompareOp::eNever;
        case gfx::DepthFunctionType::Less:              return vk::CompareOp::eLess;
        case gfx::DepthFunctionType::Equal:             return vk::CompareOp::eEqual;
        case gfx::DepthFunctionType::LessEqual:         return vk::CompareOp::eLessOrEqual;
        case gfx::DepthFunctionType::Greater:           return vk::CompareOp::eGreater;
        case gfx::DepthFunctionType::NotEqual:          return vk::CompareOp::eNotEqual;
        case gfx::DepthFunctionType::GreaterEqual:      return vk::CompareOp::eGreaterOrEqual;
        case gfx::DepthFunctionType::Always:            return vk::CompareOp::eAlways;
    }
}

void PipelineInfo::setCullMode(const gfx::CullFaceMode& value) {
    cullMode = value.enabled ? vulkanCullMode(value.side) : vk::CullModeFlagBits::eNone;
    frontFace = vulkanFrontFace(value.winding);
}

void PipelineInfo::setTopology(const gfx::DrawModeType& value) {
    topology = vulkanPrimitiveTopology(value);
}

void PipelineInfo::setColorBlend(const gfx::ColorMode& value) {
    
    setColorBlendFunction(value.blendFunction);

    if (usesBlendConstants()) {
        dynamicValues.blendConstants = value.blendColor;
    } else {
        dynamicValues.blendConstants.reset();
    }

    colorMask = vk::ColorComponentFlags();

    if (value.mask.r)   colorMask |= vk::ColorComponentFlagBits::eR;
    if (value.mask.g)   colorMask |= vk::ColorComponentFlagBits::eG;
    if (value.mask.b)   colorMask |= vk::ColorComponentFlagBits::eB;
    if (value.mask.a)   colorMask |= vk::ColorComponentFlagBits::eA;
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
        default:                                [[fallthrough]];
        case gfx::DepthMaskType::ReadOnly:      depthWrite = false; break;
        case gfx::DepthMaskType::ReadWrite:     depthWrite = true;  break;
    }
}

bool PipelineInfo::usesBlendConstants() const {
    if (srcBlendFactor == vk::BlendFactor::eConstantAlpha ||
        srcBlendFactor == vk::BlendFactor::eConstantColor ||
        srcBlendFactor == vk::BlendFactor::eOneMinusConstantAlpha ||
        srcBlendFactor == vk::BlendFactor::eOneMinusConstantColor) {
        return true;
    }

    if (dstBlendFactor == vk::BlendFactor::eConstantAlpha ||
        dstBlendFactor == vk::BlendFactor::eConstantColor ||
        dstBlendFactor == vk::BlendFactor::eOneMinusConstantAlpha ||
        dstBlendFactor == vk::BlendFactor::eOneMinusConstantColor) {
        return true;
    }

    return false;
}

std::size_t PipelineInfo::hash() const {
    return util::hash(
        topology, 
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
        wideLines
    );
}

} // namespace vulkan
} // namespace mbgl
