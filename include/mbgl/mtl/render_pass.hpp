#pragma once

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <memory>

namespace mbgl {
namespace mtl {

class CommandEncoder;
class Context;

class RenderPass final : public gfx::RenderPass {
public:
    RenderPass(CommandEncoder&, const char* name, const gfx::RenderPassDescriptor&);
    ~RenderPass() override;

    const MTLRenderCommandEncoderPtr& getMetalEncoder() const;
    const gfx::RenderPassDescriptor& getDescriptor() const { return descriptor; }

    void endEncoding();

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

private:
    gfx::RenderPassDescriptor descriptor;
    mtl::CommandEncoder& commandEncoder;
    // const gfx::DebugGroup<gfx::CommandEncoder> debugGroup;

    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mtl
} // namespace mbgl
