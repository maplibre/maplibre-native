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

CommandEncoder::~CommandEncoder() {}

std::unique_ptr<gfx::UploadPass> CommandEncoder::createUploadPass(const char* name, gfx::Renderable& renderable) {
    return std::make_unique<UploadPass>(renderable, *this, name);
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
    debugGroupNames.push_back({name ? name : std::string(), renderPasses.size(), uploadPasses.size()});

    // Apply it to any groups that are already active
    for (auto* pass : renderPasses) {
        renderDebugGroups.emplace_back(gfx::DebugGroup<gfx::RenderPass>{*pass, name});
    }
    for (auto* pass : uploadPasses) {
        uploadDebugGroups.emplace_back(gfx::DebugGroup<gfx::UploadPass>{*pass, name});
    }
}

void CommandEncoder::popDebugGroup() {
    assert(!debugGroupNames.empty());
    if (!debugGroupNames.empty()) {
        // Pop the entries we added for each active encoder when this item was added
        for (std::size_t i = 0; i < debugGroupNames.back().renderGroups; ++i) {
            assert(!renderDebugGroups.empty());
            if (!renderDebugGroups.empty()) {
                renderDebugGroups.pop_back();
            }
        }
        for (std::size_t i = 0; i < debugGroupNames.back().uploadGroups; ++i) {
            assert(!uploadDebugGroups.empty());
            if (!uploadDebugGroups.empty()) {
                uploadDebugGroups.pop_back();
            }
        }

        debugGroupNames.pop_back();
    }
}

void CommandEncoder::trackRenderPass(RenderPass* pass) {
    if (pass) {
        renderPasses.insert(pass);
    }
}

void CommandEncoder::forgetRenderPass(RenderPass* pass) {
    renderPasses.erase(pass);
}

void CommandEncoder::trackUploadPass(UploadPass* pass) {
    if (pass) {
        uploadPasses.insert(pass);
    }
}

void CommandEncoder::forgetUploadPass(UploadPass* pass) {
    uploadPasses.erase(pass);
}

} // namespace mtl
} // namespace mbgl
