#include <mbgl/shaders/mtl/shader_program.hpp>

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/mtl/vertex_attribute.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/util/logging.hpp>

#include <Metal/MTLLibrary.hpp>
#include <Metal/MTLRenderPass.hpp>
#include <Metal/MTLRenderPipeline.hpp>

#include <cstring>
#include <utility>
#include <algorithm>

using namespace std::string_literals;

namespace mbgl {

namespace mtl {
namespace {
MTL::BlendOperation metalBlendOperation(const gfx::ColorBlendEquationType& colorBlend) {
    switch (colorBlend) {
        case gfx::ColorBlendEquationType::Add:
            return MTL::BlendOperationAdd;
        case gfx::ColorBlendEquationType::Subtract:
            return MTL::BlendOperationSubtract;
        case gfx::ColorBlendEquationType::ReverseSubtract:
            return MTL::BlendOperationReverseSubtract;
    }
}

MTL::BlendFactor metalBlendFactor(const gfx::ColorBlendFactorType& colorFactor) {
    switch (colorFactor) {
        case gfx::ColorBlendFactorType::Zero:
            return MTL::BlendFactorZero;
        case gfx::ColorBlendFactorType::One:
            return MTL::BlendFactorOne;
        case gfx::ColorBlendFactorType::SrcColor:
            return MTL::BlendFactorSourceColor;
        case gfx::ColorBlendFactorType::OneMinusSrcColor:
            return MTL::BlendFactorOneMinusSourceColor;
        case gfx::ColorBlendFactorType::SrcAlpha:
            return MTL::BlendFactorSourceAlpha;
        case gfx::ColorBlendFactorType::OneMinusSrcAlpha:
            return MTL::BlendFactorOneMinusSourceAlpha;
        case gfx::ColorBlendFactorType::DstAlpha:
            return MTL::BlendFactorDestinationAlpha;
        case gfx::ColorBlendFactorType::OneMinusDstAlpha:
            return MTL::BlendFactorOneMinusDestinationAlpha;
        case gfx::ColorBlendFactorType::DstColor:
            return MTL::BlendFactorDestinationColor;
        case gfx::ColorBlendFactorType::OneMinusDstColor:
            return MTL::BlendFactorOneMinusDestinationColor;
        case gfx::ColorBlendFactorType::SrcAlphaSaturate:
            return MTL::BlendFactorSourceAlphaSaturated;
        case gfx::ColorBlendFactorType::ConstantColor:
            return MTL::BlendFactorBlendColor;
        case gfx::ColorBlendFactorType::OneMinusConstantColor:
            return MTL::BlendFactorOneMinusBlendColor;
        case gfx::ColorBlendFactorType::ConstantAlpha:
            return MTL::BlendFactorBlendAlpha;
        case gfx::ColorBlendFactorType::OneMinusConstantAlpha:
            return MTL::BlendFactorOneMinusBlendAlpha;
    }
}
} // namespace

ShaderProgram::ShaderProgram(std::string name, RendererBackend& backend_, MTLFunctionPtr vert, MTLFunctionPtr frag)
    : ShaderProgramBase(),
      shaderName(std::move(name)),
      backend(backend_),
      vertexFunction(std::move(vert)),
      fragmentFunction(std::move(frag)) {}

ShaderProgram::~ShaderProgram() noexcept = default;

MTLRenderPipelineStatePtr ShaderProgram::getRenderPipelineState(const gfx::Renderable& renderable,
                                                                const MTLVertexDescriptorPtr& vertexDescriptor,
                                                                const gfx::ColorMode& colorMode,
                                                                const std::optional<std::size_t> reuseHash) const {
    if (reuseHash.has_value()) {
        // we'd like to reuse a previous value
        if (auto it = renderPipelineStateCache.find(reuseHash.value()); it != renderPipelineStateCache.end())
            return it->second;
    }

    auto pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());

    const auto& renderableResource = renderable.getResource<RenderableResource>();

    auto colorFormat = MTL::PixelFormat::PixelFormatBGRA8Unorm;
    std::optional<MTL::PixelFormat> depthFormat = std::nullopt;
    std::optional<MTL::PixelFormat> stencilFormat = std::nullopt;
    if (const auto& rpd = renderableResource.getRenderPassDescriptor()) {
        if (auto* colorTarget = rpd->colorAttachments()->object(0)) {
            if (auto* tex = colorTarget->texture()) {
                colorFormat = tex->pixelFormat();
            }
        }
        if (auto* depthTarget = rpd->depthAttachment()) {
            if (auto* tex = depthTarget->texture()) {
                depthFormat = tex->pixelFormat();
            }
        }
        if (auto* stencilTarget = rpd->stencilAttachment()) {
            if (auto* tex = stencilTarget->texture()) {
                stencilFormat = tex->pixelFormat();
            }
        }
    }

    auto desc = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    desc->setLabel(NS::String::string(shaderName.data(), NS::UTF8StringEncoding));
    desc->setVertexFunction(vertexFunction.get());
    desc->setFragmentFunction(fragmentFunction.get());
    desc->setVertexDescriptor(vertexDescriptor.get());

    if (auto* colorTarget = desc->colorAttachments()->object(0)) {
        const auto blendEnabled = !colorMode.blendFunction.is<gfx::ColorMode::Replace>();
        auto blendOperation = MTL::BlendOperationAdd;
        auto srcFactor = MTL::BlendFactorOne;
        auto destFactor = MTL::BlendFactorOne;

        if (blendEnabled) {
            apply_visitor(
                [&](const auto& blendFunction) {
                    blendOperation = metalBlendOperation(gfx::ColorBlendEquationType(blendFunction.equation));
                    srcFactor = metalBlendFactor(blendFunction.srcFactor);
                    destFactor = metalBlendFactor(blendFunction.dstFactor);
                },
                colorMode.blendFunction);
        }

        colorTarget->setPixelFormat(colorFormat);
        colorTarget->setBlendingEnabled(blendEnabled);
        colorTarget->setRgbBlendOperation(blendOperation);
        colorTarget->setAlphaBlendOperation(blendOperation);
        colorTarget->setSourceRGBBlendFactor(srcFactor);
        colorTarget->setSourceAlphaBlendFactor(srcFactor);
        colorTarget->setDestinationRGBBlendFactor(destFactor);
        colorTarget->setDestinationAlphaBlendFactor(destFactor);

        colorTarget->setWriteMask((colorMode.mask.r ? MTL::ColorWriteMaskRed : MTL::ColorWriteMaskNone) |
                                  (colorMode.mask.g ? MTL::ColorWriteMaskGreen : MTL::ColorWriteMaskNone) |
                                  (colorMode.mask.b ? MTL::ColorWriteMaskBlue : MTL::ColorWriteMaskNone) |
                                  (colorMode.mask.a ? MTL::ColorWriteMaskAlpha : MTL::ColorWriteMaskNone));
    }

    if (depthFormat) {
        desc->setDepthAttachmentPixelFormat(*depthFormat);
    }

    if (stencilFormat) {
        desc->setStencilAttachmentPixelFormat(*stencilFormat);
    }

    NS::Error* error = nullptr;
    const auto& device = backend.getDevice();
    auto rps = NS::TransferPtr(device->newRenderPipelineState(desc.get(), &error));

    if (!rps || error) {
        const auto errPtr = error ? error->localizedDescription()->utf8String() : nullptr;
        const auto errStr = (errPtr && errPtr[0]) ? ": "s + errPtr : std::string();
        Log::Error(Event::Shader, shaderName + " newRenderPipelineState failed" + errStr);
        assert(false);
    }

    if (reuseHash.has_value()) {
        // store the value for future reuse
        renderPipelineStateCache[reuseHash.value()] = rps;
    }

    return rps;
}

std::optional<size_t> ShaderProgram::getSamplerLocation(const size_t id) const {
    return (id < textureBindings.size()) ? textureBindings[id] : std::nullopt;
}

void ShaderProgram::initAttribute(const shaders::AttributeInfo& info) {
    const auto index = static_cast<int>(info.index);
#if !defined(NDEBUG)
    // Indexes must be unique, if there's a conflict check the `attributes` array in the shader
    vertexAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { assert(attrib.getIndex() != index); });
    instanceAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { assert(attrib.getIndex() != index); });
#endif
    vertexAttributes.set(info.id, index, info.dataType, 1);
}

void ShaderProgram::initInstanceAttribute(const shaders::AttributeInfo& info) {
    // Index is the block index of the instance attribute
    const auto index = static_cast<int>(info.index);
#if !defined(NDEBUG)
    // Indexes must not be reused by regular attributes or uniform blocks
    // More than one instance attribute can have the same index, if they share the block
    vertexAttributes.visitAttributes([&](const gfx::VertexAttribute& attrib) { assert(attrib.getIndex() != index); });
#endif
    instanceAttributes.set(info.id, index, info.dataType, 1);
}

void ShaderProgram::initTexture(const shaders::TextureInfo& info) {
    assert(info.id < textureBindings.size());
    if (info.id >= textureBindings.size()) {
        return;
    }
    textureBindings[info.id] = info.index;
}

} // namespace mtl
} // namespace mbgl
