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
    : context(context_) {}

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

void CommandEncoder::pushDebugGroup(const char* name) {
    // Debug groups exist in MTLBuffer and MTLCommandEncoder, but these are associated with the
    // renderable provided in the RenderPassDescriptor, and so may not have been created yet.
    // Instead, maintain a stack of names and push them to each render pass as we create it.
    debugGroupNames.push_back(std::make_pair(name ? name : std::string(), passes.size()));

    // Apply it to any groups that are already active
    for (auto* pass : passes) {
        debugGroups.emplace_back(gfx::DebugGroup<gfx::RenderPass>{*pass, name});
    }
}

void CommandEncoder::popDebugGroup() {
    assert(!debugGroupNames.empty());
    if (!debugGroupNames.empty()) {
        // Pop the entries we added for each active encoder when this item was added
        const auto popCount = debugGroupNames.back().second;
        for (std::size_t i = 0; i < popCount; ++i) {
            assert(!debugGroups.empty());
            if (!debugGroups.empty()) {
                debugGroups.pop_back();
            }
        }

        debugGroupNames.pop_back();
    }
}

void CommandEncoder::trackRenderPass(RenderPass* pass) {
    if (pass) {
        passes.insert(pass);
    }
}

void CommandEncoder::forgetRenderPass(RenderPass* pass) {
    passes.erase(pass);
}

} // namespace mtl
} // namespace mbgl
