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

#include <Metal/Metal.hpp>

#include <cstring>
#include <utility>

namespace mbgl {
namespace mtl {

struct ShaderProgram::Impl {
    Impl(MTLFunctionPtr&& vert,
         MTLFunctionPtr&& frag) :
        vertexFunction(std::move(vert)),
        fragmentFunction(std::move(frag)) {}

    MTLFunctionPtr vertexFunction;
    MTLFunctionPtr fragmentFunction;
};

ShaderProgram::ShaderProgram(std::string name,
                             RendererBackend& backend_,
                             MTLFunctionPtr vertexFunction,
                             MTLFunctionPtr fragmentFunction)
    : ShaderProgramBase(),
      shaderName(std::move(name)),
      backend(backend_),
      impl(std::make_unique<Impl>(std::move(vertexFunction),
                                  std::move(fragmentFunction))) {
}

ShaderProgram::~ShaderProgram() noexcept = default;

MTLRenderPipelineStatePtr ShaderProgram::getRenderPipelineState(const gfx::RenderPassDescriptor& descriptor) const {
    auto pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());

    const auto& renderable = descriptor.renderable;
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

    auto* desc = MTL::RenderPipelineDescriptor::alloc()->init();
    desc->setLabel(NS::String::string(shaderName.data(), NS::UTF8StringEncoding));
    desc->setVertexFunction(impl->vertexFunction.get());
    desc->setFragmentFunction(impl->fragmentFunction.get());

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
    auto rps = NS::RetainPtr(device->newRenderPipelineState(desc, &error));

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

namespace {
static UniformBlockArray noUniforms;
static VertexAttributeArray noAttribs;
}
const gfx::UniformBlockArray& ShaderProgram::getUniformBlocks() const {
    return noUniforms;
}

const gfx::VertexAttributeArray& ShaderProgram::getVertexAttributes() const {
    return noAttribs;
}

gfx::UniformBlockArray& ShaderProgram::mutableUniformBlocks() {
    return noUniforms;
}

gfx::VertexAttributeArray& ShaderProgram::mutableVertexAttributes() {
    return noAttribs;
}

} // namespace mtl
} // namespace mbgl
