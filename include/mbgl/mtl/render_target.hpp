#pragma once

#include <mbgl/renderer/render_target.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/util/size.hpp>

namespace mbgl {
namespace mtl {

class Context;
// using UniqueFramebufferPtr = std::shared_ptr<UniqueFramebuffer>;

class RenderTarget final : public mbgl::RenderTarget {
public:
    RenderTarget(Context& context_, const Size size, const gfx::TextureChannelDataType type);
    ~RenderTarget() override;

    void upload(gfx::UploadPass& uploadPass) override;
    void render(RenderOrchestrator&, const RenderTree&, PaintParameters&) override;

private:
    Context& context;
    // UniqueFramebufferPtr framebuffer;
};

} // namespace mtl
} // namespace mbgl
