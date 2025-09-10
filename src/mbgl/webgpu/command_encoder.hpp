#pragma once

#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/gfx/debug_group.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/util/color.hpp>

namespace mbgl {
namespace webgpu {

class Context;

class CommandEncoder : public gfx::CommandEncoder {
public:
    explicit CommandEncoder(Context& context);
    ~CommandEncoder() override;
    
    void setDepthMode(const gfx::DepthMode&) override;
    void setStencilMode(const gfx::StencilMode&) override;
    void setColorMode(const gfx::ColorMode&) override;
    void setCullFaceMode(const gfx::CullFaceMode&) override;
    void setDrawableProvider(gfx::DrawableProvider*) override;
    void setScissorTest(bool) override;
    void setScissorRect(const gfx::ScissorTest&) override;
    void clearColor(const Color&) override;
    void clearStencil(int32_t) override;
    void clearDepth(float) override;
    void setViewport(const gfx::Viewport&) override;
    void draw(const gfx::DrawablePtr&) override;
    
    std::unique_ptr<gfx::RenderPass> createRenderPass(const char*, const gfx::RenderPassDescriptor&) override;
    std::unique_ptr<gfx::UploadPass> createUploadPass(const char*) override;
    
    void present(gfx::Renderable&) override;
    void finish() override;
    
    gfx::Context& getContext();
    const gfx::Context& getContext() const;
    
    std::unique_ptr<gfx::DebugGroup<gfx::CommandEncoder>> createDebugGroup(const char* name);

private:
    Context& context_;
};

} // namespace webgpu
} // namespace mbgl