#pragma once

#include <mbgl/gfx/render_pass.hpp>

namespace mbgl {
namespace gfx {

class CommandEncoder;

} // namespace gfx

namespace mtl {

class CommandEncoder;
class Context;

class RenderPass final : public gfx::RenderPass {
public:
    RenderPass(CommandEncoder&, const char* name, const gfx::RenderPassDescriptor&);

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

private:
    mtl::CommandEncoder& commandEncoder;
    //const gfx::DebugGroup<gfx::CommandEncoder> debugGroup;
};

} // namespace mtl
} // namespace mbgl
