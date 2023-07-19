#include <mbgl/shaders/mtl/shader_program.hpp>

#include <mbgl/mtl/context.hpp>
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
    Impl(MTLDevicePtr&& device_,
         MTLFunctionPtr&& vert,
         MTLFunctionPtr&& frag) :
        device(std::move(device_)),
        vertexFunction(std::move(vert)),
        fragmentFunction(std::move(frag)) {}

    MTLDevicePtr device;
    MTLFunctionPtr vertexFunction;
    MTLFunctionPtr fragmentFunction;
    MTLRenderPipelineStatePtr renderPipelineState;
};

ShaderProgram::ShaderProgram(std::string name,
                             MTLDevicePtr device_,
                             MTLFunctionPtr vertexFunction,
                             MTLFunctionPtr fragmentFunction)
    : ShaderProgramBase(),
      shaderName(std::move(name)),
      impl(std::make_unique<Impl>(std::move(device_),
                                  std::move(vertexFunction),
                                  std::move(fragmentFunction))) {
}

ShaderProgram::~ShaderProgram() noexcept = default;

const MTLRenderPipelineStatePtr& ShaderProgram::getRenderPipelineState() const {
    constexpr auto pixelFormat = MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB;

    if (!impl->renderPipelineState) {
        auto pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());
        
        auto* desc = MTL::RenderPipelineDescriptor::alloc()->init();
        desc->setLabel(NS::String::string(shaderName.data(), NS::UTF8StringEncoding));
        desc->setVertexFunction(impl->vertexFunction.get());
        desc->setFragmentFunction(impl->fragmentFunction.get());
        desc->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
        
        NS::Error* error = nullptr;
        impl->renderPipelineState = NS::RetainPtr(impl->device->newRenderPipelineState(desc, &error));
        
        if (!impl->renderPipelineState || error) {
            const auto errPtr = error ? error->localizedDescription()->utf8String() : nullptr;
            const auto errStr = (errPtr && errPtr[0]) ? ": " + std::string(errPtr) : std::string();
            Log::Error(Event::Shader, shaderName + " newRenderPipelineState failed" + errStr);
            assert(false);
        }
    }

    return impl->renderPipelineState;
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
