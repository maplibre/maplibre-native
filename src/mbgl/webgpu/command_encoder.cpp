#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/upload_pass.hpp>

namespace mbgl {
namespace webgpu {

CommandEncoder::CommandEncoder(Context& ctx) 
    : context(ctx) {}

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

void CommandEncoder::trackRenderPass(RenderPass* renderPass) {
    renderPasses.insert(renderPass);
    // TODO: Propagate debug groups to render pass
}

void CommandEncoder::forgetRenderPass(RenderPass* renderPass) {
    renderPasses.erase(renderPass);
}

void CommandEncoder::trackUploadPass(UploadPass* uploadPass) {
    uploadPasses.insert(uploadPass);
    // TODO: Propagate debug groups to upload pass
}

void CommandEncoder::forgetUploadPass(UploadPass* uploadPass) {
    uploadPasses.erase(uploadPass);
}

void CommandEncoder::pushDebugGroup(const char* name) {
    debugGroupNames.push_back({name, 0, 0});
    
    // Propagate to tracked render passes
    for (auto* pass : renderPasses) {
        (void)pass; // TODO: Push debug group to render pass
    }
    
    // Propagate to tracked upload passes
    for (auto* pass : uploadPasses) {
        (void)pass; // TODO: Push debug group to upload pass
    }
}

void CommandEncoder::popDebugGroup() {
    if (!debugGroupNames.empty()) {
        debugGroupNames.pop_back();
        
        // Propagate to tracked render passes
        for (auto* pass : renderPasses) {
            (void)pass; // TODO: Pop debug group from render pass
        }
        
        // Propagate to tracked upload passes
        for (auto* pass : uploadPasses) {
            (void)pass; // TODO: Pop debug group from upload pass
        }
    }
}

} // namespace webgpu
} // namespace mbgl