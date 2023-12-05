#pragma once

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLCommandEncoder.hpp>

#include <memory>
#include <optional>

namespace mbgl {
namespace mtl {

class BufferResource;
class CommandEncoder;
class Context;

class RenderPass final : public gfx::RenderPass {
public:
    RenderPass(CommandEncoder&, const char* name, const gfx::RenderPassDescriptor&);
    ~RenderPass() override;

    mtl::CommandEncoder& getCommandEncoder() { return commandEncoder; }
    const mtl::CommandEncoder& getCommandEncoder() const { return commandEncoder; }

    const MTLRenderCommandEncoderPtr& getMetalEncoder() const { return encoder; }
    const gfx::RenderPassDescriptor& getDescriptor() const { return descriptor; }

    void endEncoding();

    void addDebugSignpost(const char* name) override;

    void bindVertex(const BufferResource&, std::size_t offset, std::size_t index, std::size_t size = 0);

    void bindFragment(const BufferResource&, std::size_t offset, std::size_t index, std::size_t size = 0);

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

private:
    gfx::RenderPassDescriptor descriptor;
    mtl::CommandEncoder& commandEncoder;
    MTLRenderCommandEncoderPtr encoder;
    std::vector<gfx::DebugGroup<gfx::RenderPass>> debugGroups;

    struct BindInfo {
        const BufferResource* buf = nullptr;
        NS::UInteger offset = 0;
        std::uint16_t version = 0;
    };
    static constexpr auto maxBinds = 32;
    std::array<std::optional<BindInfo>, maxBinds> vertexBinds;
    std::array<std::optional<BindInfo>, maxBinds> fragmentBinds;
};

} // namespace mtl
} // namespace mbgl
