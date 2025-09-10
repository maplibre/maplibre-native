#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/upload_pass.hpp>

namespace mbgl {
namespace webgpu {

CommandEncoder::CommandEncoder(Context& context) 
    : context_(context) {
}

CommandEncoder::~CommandEncoder() = default;

void CommandEncoder::setDepthMode(const gfx::DepthMode&) {}
void CommandEncoder::setStencilMode(const gfx::StencilMode&) {}
void CommandEncoder::setColorMode(const gfx::ColorMode&) {}
void CommandEncoder::setCullFaceMode(const gfx::CullFaceMode&) {}
void CommandEncoder::setDrawableProvider(gfx::DrawableProvider*) {}
void CommandEncoder::setScissorTest(bool) {}
void CommandEncoder::setScissorRect(const gfx::ScissorTest&) {}
void CommandEncoder::clearColor(const Color&) {}
void CommandEncoder::clearStencil(int32_t) {}
void CommandEncoder::clearDepth(float) {}
void CommandEncoder::setViewport(const gfx::Viewport&) {}
void CommandEncoder::draw(const gfx::DrawablePtr&) {}

std::unique_ptr<gfx::RenderPass> CommandEncoder::createRenderPass(const char* name, 
                                                                  const gfx::RenderPassDescriptor& descriptor) {
    return std::make_unique<RenderPass>(*this, name, descriptor);
}

std::unique_ptr<gfx::UploadPass> CommandEncoder::createUploadPass(const /*char* name*/) {
    return std::make_unique<UploadPass>(*this, name);
}

void CommandEncoder::present(gfx::Renderable&) {
    // TODO: Present to WebGPU surface
}

void CommandEncoder::finish() {
    // TODO: Finish WebGPU command encoding
}

gfx::Context& CommandEncoder::getContext() {
    return context_;
}

const gfx::Context& CommandEncoder::getContext() const {
    return context_;
}

std::unique_ptr<gfx::DebugGroup<gfx::CommandEncoder>> CommandEncoder::createDebugGroup(const char* name) {
    return std::make_unique<gfx::DebugGroup<gfx::CommandEncoder>>(*this, name);
}

} // namespace webgpu
} // namespace mbgl