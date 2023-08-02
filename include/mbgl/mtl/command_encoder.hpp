#pragma once

#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace mbgl {
namespace mtl {

class Context;
class RenderPass;

class CommandEncoder final : public gfx::CommandEncoder {
public:
    explicit CommandEncoder(Context& context_);
    ~CommandEncoder() override;

    std::unique_ptr<gfx::UploadPass> createUploadPass(const char* name) override;
    std::unique_ptr<gfx::RenderPass> createRenderPass(const char* name, const gfx::RenderPassDescriptor&) override;

    void present(gfx::Renderable&) override;

    template <typename TFunc>
    void visitDebugGroups(TFunc f) {
        for (const auto& item : debugGroupNames) {
            f(item.first);
        }
    }

private:
    friend class RenderPass;

    // Track a RenderPass, propagating debug groups to it
    void trackRenderPass(RenderPass*);
    // Stop tracking a RenderPass
    void forgetRenderPass(RenderPass*);

    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

public:
    mtl::Context& context;
    std::vector<std::pair<std::string, std::size_t>> debugGroupNames;
    std::vector<gfx::DebugGroup<gfx::RenderPass>> debugGroups;
    std::unordered_set<RenderPass*> passes;
};

} // namespace mtl
} // namespace mbgl
