#pragma once

#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace mbgl {
namespace gfx {
class Renderable;
} // namespace gfx
namespace mtl {

class Context;
class RenderPass;
class UploadPass;

class CommandEncoder final : public gfx::CommandEncoder {
public:
    explicit CommandEncoder(Context& context_);
    ~CommandEncoder() override;

    std::unique_ptr<gfx::UploadPass> createUploadPass(const char* name, gfx::Renderable&) override;
    std::unique_ptr<gfx::RenderPass> createRenderPass(const char* name, const gfx::RenderPassDescriptor&) override;

    void present(gfx::Renderable&) override;

    template <typename TFunc>
    void visitDebugGroups(TFunc f) {
        for (const auto& item : debugGroupNames) {
            f(item.name);
        }
    }

private:
    friend class RenderPass;
    friend class UploadPass;

    // Track a RenderPass, propagating debug groups to it
    void trackRenderPass(RenderPass*);
    // Stop tracking a RenderPass
    void forgetRenderPass(RenderPass*);

    // Track an UploadPass, propagating debug groups to it
    void trackUploadPass(UploadPass*);
    // Stop tracking a RenderPass
    void forgetUploadPass(UploadPass*);

    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

protected:
    struct GroupEntry {
        std::string name;
        std::size_t renderGroups;
        std::size_t uploadGroups;
    };
protected:
    mtl::Context& context;
    std::vector<GroupEntry> debugGroupNames;
    std::vector<gfx::DebugGroup<gfx::RenderPass>> renderDebugGroups;
    std::vector<gfx::DebugGroup<gfx::UploadPass>> uploadDebugGroups;
    std::unordered_set<RenderPass*> renderPasses;
    std::unordered_set<UploadPass*> uploadPasses;
};

} // namespace mtl
} // namespace mbgl
