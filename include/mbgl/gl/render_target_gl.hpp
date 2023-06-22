#pragma once

#include <mbgl/renderer/render_target.hpp>
#include <mbgl/gl/types.hpp>

namespace mbgl {
namespace gl {

class Context;
class Framebuffer;
using FramebufferPtr = std::shared_ptr<Framebuffer>;

class RenderTargetGL final : public RenderTarget {
public:
    RenderTargetGL(Context& context);
    ~RenderTargetGL() override;
    
    void render(RenderOrchestrator&, const RenderTree&, PaintParameters&) override;

private:
    Context& glContext;
    FramebufferID id = 0;
};

} // namespace gl
} // namespace mbgl
