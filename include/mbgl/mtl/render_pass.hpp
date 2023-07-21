#pragma once

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLCommandEncoder.hpp>

#include <memory>

namespace mbgl {
namespace mtl {

class CommandEncoder;
class Context;

class RenderPass final : public gfx::RenderPass {
public:
    RenderPass(CommandEncoder&, const char* name, const gfx::RenderPassDescriptor&);
    ~RenderPass() override;

    const MTLRenderCommandEncoderPtr& getMetalEncoder() const { return encoder; }
    const gfx::RenderPassDescriptor& getDescriptor() const { return descriptor; }

    void endEncoding();

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

private:
    gfx::RenderPassDescriptor descriptor;
    mtl::CommandEncoder& commandEncoder;
    MTLRenderCommandEncoderPtr encoder;
    // const gfx::DebugGroup<gfx::CommandEncoder> debugGroup;
};

} // namespace mtl
} // namespace mbgl
