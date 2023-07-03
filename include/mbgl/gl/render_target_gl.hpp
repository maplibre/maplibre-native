#pragma once

#include <mbgl/renderer/render_target.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/gl/object.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/util/size.hpp>

namespace mbgl {
namespace gl {

class Context;
using UniqueFramebufferPtr = std::shared_ptr<UniqueFramebuffer>;

class RenderTargetGL final : public RenderTarget {
public:
    RenderTargetGL(Context& context, const Size size, const gfx::TextureChannelDataType type);
    ~RenderTargetGL() override;

    void upload(gfx::UploadPass& uploadPass) override;
    void render(RenderOrchestrator&, const RenderTree&, PaintParameters&) override;

private:
    Context& glContext;
    UniqueFramebufferPtr framebuffer;
};

} // namespace gl
} // namespace mbgl
