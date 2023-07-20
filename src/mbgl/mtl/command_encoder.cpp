#include <mbgl/mtl/command_encoder.hpp>

#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/upload_pass.hpp>

#include <Metal/Metal.hpp>

#include <cstring>

namespace mbgl {
namespace mtl {

CommandEncoder::CommandEncoder(Context& context_)
    : context(context_) {
}

CommandEncoder::~CommandEncoder() {
    context.performCleanup();
}

std::unique_ptr<gfx::UploadPass> CommandEncoder::createUploadPass(const char* name) {
    return std::make_unique<UploadPass>(*this, name);
}

std::unique_ptr<gfx::RenderPass> CommandEncoder::createRenderPass(const char* name,
                                                                  const gfx::RenderPassDescriptor& descriptor) {
    return std::make_unique<RenderPass>(*this, name, descriptor);
}

void CommandEncoder::present(gfx::Renderable& renderable) {
    renderable.getResource<RenderableResource>().swap();
}


void CommandEncoder::pushDebugGroup(const char* /*name*/) {
}

void CommandEncoder::popDebugGroup() {
}

} // namespace mtl
} // namespace mbgl
