#include <mbgl/shaders/mtl/shader_program.hpp>

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/uniform_block.hpp>
#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/mtl/vertex_attribute.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/util/logging.hpp>

#include <Metal/MTLRenderPass.hpp>
#include <Metal/MTLRenderPipeline.hpp>

#include <cstring>
#include <utility>

namespace mbgl {
namespace mtl {

ShaderProgram::ShaderProgram(std::string name, RendererBackend& backend_, MTLFunctionPtr vert, MTLFunctionPtr frag)
    : ShaderProgramBase(),
      shaderName(std::move(name)),
      backend(backend_),
      vertexFunction(std::move(vert)),
      fragmentFunction(std::move(frag)) {}

MTLRenderPipelineStatePtr ShaderProgram::getRenderPipelineState(const gfx::RenderPassDescriptor& renderPassDescriptor,
                                                                const MTLVertexDescriptorPtr& vertexDescriptor) const {
    auto pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());

    const auto& renderable = renderPassDescriptor.renderable;
    const auto& renderableResource = renderable.getResource<RenderableResource>();

    auto colorFormat = MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB;
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
        colorTarget->setPixelFormat(colorFormat);
        colorTarget->setBlendingEnabled(true);
        colorTarget->setRgbBlendOperation(MTL::BlendOperationAdd);
        colorTarget->setAlphaBlendOperation(MTL::BlendOperationAdd);
        colorTarget->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
        colorTarget->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
        colorTarget->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        colorTarget->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
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
        const auto errStr = (errPtr && errPtr[0]) ? ": " + std::string(errPtr) : std::string();
        Log::Error(Event::Shader, shaderName + " newRenderPipelineState failed" + errStr);
        assert(false);
    }

    return rps;
}

std::optional<uint32_t> ShaderProgram::getSamplerLocation(std::string_view name) const {
    return std::nullopt;
}

void ShaderProgram::initAttribute(const shaders::AttributeInfo& info) {
    vertexAttributes.add(std::string(info.name), static_cast<int>(info.index), info.dataType, info.count);
}

void ShaderProgram::initUniformBlock(const shaders::UniformBlockInfo& info) {
    if (const auto& block_ = uniformBlocks.add(info.name.data(), static_cast<int>(info.index), info.size)) {
        auto& block = static_cast<UniformBlock&>(*block_);
        block.setBindVertex(info.vertex);
        block.setBindFragment(info.fragment);
    }
}

} // namespace mtl
} // namespace mbgl
