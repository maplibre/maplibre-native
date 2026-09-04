#pragma once

#include <mln/gfx/render_pass.hpp>

namespace mln {
namespace gfx {

class CommandEncoder;

} // namespace gfx

namespace gl {

class CommandEncoder;
class Context;

class RenderPass final : public gfx::RenderPass {
public:
    RenderPass(gl::CommandEncoder&, const char* name, const gfx::RenderPassDescriptor&);
    ~RenderPass() override;

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

private:
    gl::CommandEncoder& commandEncoder;
    const gfx::DebugGroup<gfx::CommandEncoder> debugGroup;
    // Whether this pass's target has a depth / stencil attachment (from the
    // descriptor's clear values), so the destructor can invalidate them.
    const bool hasDepth;
    const bool hasStencil;
};

} // namespace gl
} // namespace mln
