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

std::unique_ptr<gfx::RenderPass> CommandEncoder::createRenderPass(const char* name, 
                                                                  const gfx::RenderPassDescriptor& descriptor) {
    return std::make_unique<RenderPass>(*this, name, descriptor);
}

std::unique_ptr<gfx::UploadPass> CommandEncoder::createUploadPass(const char* name, gfx::Renderable&) {
    return std::make_unique<UploadPass>(*this, name);
}

void CommandEncoder::present(gfx::Renderable&) {
    // TODO: Present to WebGPU surface
}

void CommandEncoder::pushDebugGroup(const char*) {
    // TODO: Push WebGPU debug group
}

void CommandEncoder::popDebugGroup() {
    // TODO: Pop WebGPU debug group
}

} // namespace webgpu
} // namespace mbgl