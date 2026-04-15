#pragma once

#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLCommandEncoder.hpp>
#include <Metal/Metal.hpp>

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

    /// Apply the given depth/stencil state, if different from the current value
    /// The state may be null, restoring the default state.
    void setDepthStencilState(const MTLDepthStencilStatePtr&);

    /// Apply the given stencil reference value, if different from the current value
    void setStencilReference(int32_t referenceValue);

    /// Bind a texture to the fragment location
    void setFragmentTexture(const MTLTexturePtr&, int32_t location);

    /// Set the sampler for a texture binding
    void setFragmentSamplerState(const MTLSamplerStatePtr&, int32_t location);

    /// Set the render pipeline state
    void setRenderPipelineState(const MTLRenderPipelineStatePtr&);

    void endEncoding();

    /// Resets the states and bindings of the render pass to deal
    /// with situations where they are changed by calling the
    /// underlying rendering API directly. Currently this happens
    /// when a user changes one of these states or bindings
    /// in a custom layer
    void resetState();

    void addDebugSignpost(const char* name) override;

    void bindVertex(const BufferResource&, std::size_t offset, std::size_t index, std::size_t size = 0);
    void unbindVertex(std::size_t index);
    void bindFragment(const BufferResource&, std::size_t offset, std::size_t index, std::size_t size = 0);
    void unbindFragment(std::size_t index);

    void setCullMode(const MTL::CullMode);
    void setFrontFacingWinding(const MTL::Winding);
    void setScissorRect(const MTL::ScissorRect);

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

private:
    gfx::RenderPassDescriptor descriptor;
    mtl::CommandEncoder& commandEncoder;
    MTLRenderCommandEncoderPtr encoder;
    MTLDepthStencilStatePtr currentDepthStencilState;
    MTLRenderPipelineStatePtr currentPipelineState;

    int32_t currentStencilReferenceValue = 0;
    std::vector<gfx::DebugGroup<gfx::RenderPass>> debugGroups;

    struct BindInfo {
        const BufferResource* buf = nullptr;
        NS::UInteger size = 0;
        NS::UInteger offset = 0;
        std::uint16_t version = 0;
    };
    static constexpr auto maxBinds = 32;
    std::array<std::optional<BindInfo>, maxBinds> vertexBinds;
    std::array<std::optional<BindInfo>, maxBinds> fragmentBinds;

    std::array<MTLTexturePtr, maxBinds> fragmentTextureBindings;
    std::array<MTLSamplerStatePtr, maxBinds> fragmentSamplerStates;

    MTL::CullMode currentCullMode = MTL::CullModeNone;
    MTL::Winding currentWinding = MTL::WindingClockwise;
    MTL::ScissorRect currentScissorRect;

    size_t width;
    size_t height;
};

} // namespace mtl
} // namespace mbgl
