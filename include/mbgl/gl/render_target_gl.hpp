#pragma once

#include <mbgl/renderer/render_target.hpp>
#include <mbgl/gl/object.hpp>
#include <mbgl/gl/types.hpp>

namespace mbgl {
namespace gl {

class Context;
using UniqueFramebufferPtr = std::shared_ptr<UniqueFramebuffer>;

class RenderTargetGL final : public RenderTarget {
public:
    RenderTargetGL(Context& context);
    ~RenderTargetGL() override;
    
    void upload(gfx::UploadPass& uploadPass) override;
    void render(RenderOrchestrator&, const RenderTree&, PaintParameters&) override;

private:
    [[maybe_unused]] Context& glContext;
    UniqueFramebufferPtr framebuffer;
};

} // namespace gl
} // namespace mbgl
