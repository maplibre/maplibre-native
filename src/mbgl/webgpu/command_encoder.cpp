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

void CommandEncoder::present(gfx::Renderable& renderable) {
    // Submit command buffer and present the surface
    auto device = context.getDevice();
    auto queue = context.getQueue();
    
    if (!device || !queue) {
        return;
    }
    
    // Get the command encoder from context
    WGPUCommandEncoder cmdEncoder = context.getCommandEncoder();
    if (!cmdEncoder) {
        return;
    }
    
    // Finish and submit the command buffer
    WGPUCommandBufferDescriptor cmdBufferDesc = {};
    cmdBufferDesc.label = "Command Buffer";
    WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(cmdEncoder, &cmdBufferDesc);
    
    if (cmdBuffer) {
        wgpuQueueSubmit(queue, 1, &cmdBuffer);
        wgpuCommandBufferRelease(cmdBuffer);
    }
    
    // Present the surface if applicable
    // Note: The actual surface presentation depends on the platform integration
    (void)renderable;
}

void CommandEncoder::trackRenderPass(RenderPass* renderPass) {
    renderPasses.insert(renderPass);
    
    // Propagate existing debug groups to the new render pass
    if (!debugGroupNames.empty() && renderPass) {
        // WebGPU doesn't have a direct way to push debug groups to an existing pass
        // This would typically be handled when creating the render pass encoder
    }
}

void CommandEncoder::forgetRenderPass(RenderPass* renderPass) {
    renderPasses.erase(renderPass);
}

void CommandEncoder::trackUploadPass(UploadPass* uploadPass) {
    uploadPasses.insert(uploadPass);
    
    // Propagate existing debug groups to the new upload pass
    if (!debugGroupNames.empty() && uploadPass) {
        // WebGPU doesn't have a direct way to push debug groups to an existing pass
        // This would typically be handled when creating the upload pass
    }
}

void CommandEncoder::forgetUploadPass(UploadPass* uploadPass) {
    uploadPasses.erase(uploadPass);
}

void CommandEncoder::pushDebugGroup(const char* name) {
    debugGroupNames.push_back({name, 0, 0});
    
    // Get the WebGPU command encoder
    WGPUCommandEncoder cmdEncoder = context.getCommandEncoder();
    if (cmdEncoder) {
        wgpuCommandEncoderPushDebugGroup(cmdEncoder, name);
    }
    
    // Note: Individual render/upload passes handle their own debug groups
    // when they create their respective encoders
}

void CommandEncoder::popDebugGroup() {
    if (!debugGroupNames.empty()) {
        debugGroupNames.pop_back();
        
        // Get the WebGPU command encoder
        WGPUCommandEncoder cmdEncoder = context.getCommandEncoder();
        if (cmdEncoder) {
            wgpuCommandEncoderPopDebugGroup(cmdEncoder);
        }
        
        // Note: Individual render/upload passes handle their own debug groups
        // when they create their respective encoders
    }
}

} // namespace webgpu
} // namespace mbgl