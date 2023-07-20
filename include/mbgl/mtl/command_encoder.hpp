#pragma once

#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <memory>

namespace mbgl {
namespace mtl {

class Context;

class CommandEncoder final : public gfx::CommandEncoder {
public:
    explicit CommandEncoder(Context& context_);
    ~CommandEncoder() override;

    std::unique_ptr<gfx::UploadPass> createUploadPass(const char* name) override;
    std::unique_ptr<gfx::RenderPass> createRenderPass(const char* name, const gfx::RenderPassDescriptor&) override;

    void present(gfx::Renderable&) override;

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

public:
    mtl::Context& context;
};

} // namespace mtl
} // namespace mbgl

