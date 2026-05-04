#pragma once

#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/util/containers.hpp>
#include <webgpu/webgpu.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace mbgl {
namespace gfx {
class Renderable;
} // namespace gfx

namespace webgpu {

class Context;
class RenderPass;
class UploadPass;

class CommandEncoder final : public gfx::CommandEncoder {
public:
    explicit CommandEncoder(Context& context_);
    ~CommandEncoder() override;

    webgpu::Context& getContext() { return context; }
    const webgpu::Context& getContext() const { return context; }
    WGPUCommandEncoder getEncoder() const { return encoder; }

    std::unique_ptr<gfx::UploadPass> createUploadPass(const char* name, gfx::Renderable&) override;
    std::unique_ptr<gfx::RenderPass> createRenderPass(const char* name, const gfx::RenderPassDescriptor&) override;

    void present(gfx::Renderable&) override;

    // WebGPU-specific: submit command buffer directly
    void submitCommandBuffer();

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
    webgpu::Context& context;
    WGPUCommandEncoder encoder = nullptr;
    std::vector<GroupEntry> debugGroupNames;
    std::vector<gfx::DebugGroup<gfx::RenderPass>> renderDebugGroups;
    std::vector<gfx::DebugGroup<gfx::UploadPass>> uploadDebugGroups;
    mbgl::unordered_set<RenderPass*> renderPasses;
    mbgl::unordered_set<UploadPass*> uploadPasses;
    UploadPass* currentUploadPass = nullptr;
};

} // namespace webgpu
} // namespace mbgl
