#include <mbgl/mtl/render_pass.hpp>

#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/util/logging.hpp>

#include <Metal/Metal.hpp>

namespace mbgl {
namespace mtl {

RenderPass::RenderPass(CommandEncoder& commandEncoder_, const char* name, const gfx::RenderPassDescriptor& descriptor)
    : descriptor(descriptor),
      commandEncoder(commandEncoder_) {
    auto& resource = descriptor.renderable.getResource<RenderableResource>();

    resource.bind();

    if (const auto& buffer = resource.getCommandBuffer()) {
        if (auto rpd = resource.getRenderPassDescriptor()) {
            if (descriptor.clearColor) {
                if (auto copy = NS::TransferPtr(rpd->copy())) {
                    if (auto* colorTarget = copy->colorAttachments()->object(0)) {
                        const auto& c = *descriptor.clearColor;
                        colorTarget->setLoadAction(MTL::LoadActionClear);
                        colorTarget->setClearColor(MTL::ClearColor::Make(c.r, c.g, c.b, c.a));
                        rpd = std::move(copy);
                    }
                }
            }
            encoder = NS::RetainPtr(buffer->renderCommandEncoder(rpd.get()));

            const auto& texture = rpd->colorAttachments()->object(0)->texture();
            width = texture->width();
            height = texture->height();
        }
    }

    assert(encoder);

    // Push the groups already accumulated by the encoder
    commandEncoder.visitDebugGroups([this](const auto& group) {
        debugGroups.emplace_back(gfx::DebugGroup<gfx::RenderPass>{*this, group.c_str()});
    });

    // Push the group for the name provided
    debugGroups.emplace_back(gfx::DebugGroup<gfx::RenderPass>{*this, name});

    // Let the encoder pass along any groups pushed to it after this
    commandEncoder.trackRenderPass(this);

    commandEncoder.context.performCleanup();
}

RenderPass::~RenderPass() {
    commandEncoder.forgetRenderPass(this);
    endEncoding();
}

void RenderPass::endEncoding() {
    debugGroups.clear();

    if (encoder) {
        encoder->endEncoding();
        encoder.reset();
    }

    resetState();
}

void RenderPass::resetState() {
    currentPipelineState.reset();
    currentDepthStencilState.reset();
    currentStencilReferenceValue = 0;
    for (int i = 0; i < maxBinds; ++i) {
        vertexBinds[i].reset();
        fragmentBinds[i].reset();
        fragmentTextureBindings[i].reset();
        fragmentSamplerStates[i].reset();
    }

    currentCullMode = MTL::CullModeNone;
    currentWinding = MTL::WindingClockwise;
    currentScissorRect = {0, 0, 0, 0};
}

namespace {
constexpr auto missing = "<none>";
NS::String* toNSString(const char* str) {
    return NS::String::string(str ? str : missing, NS::UTF8StringEncoding);
}
} // namespace

void RenderPass::pushDebugGroup(const char* name) {
    assert(encoder);
    if (encoder) {
        encoder->pushDebugGroup(toNSString(name));
    }
}

void RenderPass::popDebugGroup() {
    assert(encoder);
    if (encoder) {
        encoder->popDebugGroup();
    }
}

void RenderPass::addDebugSignpost(const char* name) {
    assert(encoder);
    if (encoder) {
        encoder->insertDebugSignpost(toNSString(name));
    }
}

void RenderPass::bindVertex(const BufferResource& buf, std::size_t offset, std::size_t index, std::size_t size) {
    const auto actualSize = size ? size : buf.getSizeInBytes() - offset;
    assert(actualSize <= buf.getSizeInBytes());
    assert(0 <= index && index < maxBinds);
    if (0 <= index && index < maxBinds) {
        if (auto& bind = vertexBinds[index]) {
            // Is this the same buffer already bound to this index?
            if (bind->buf == &buf && !buf.needReBind(bind->version)) {
                // Yes, but is the offset different?
                if (bind->offset != offset) {
                    // Yes, update just the offset
                    buf.updateVertexBindOffset(encoder, offset, index, actualSize);
                    bind->offset = offset;
                }
                return;
            }
        }
        vertexBinds[index] = BindInfo{&buf, actualSize, offset};
    }
    buf.bindVertex(encoder, offset, index, actualSize);
}

void RenderPass::unbindVertex(std::size_t index) {
    vertexBinds[index] = std::nullopt;
}

void RenderPass::bindFragment(const BufferResource& buf, std::size_t offset, std::size_t index, std::size_t size) {
    const auto actualSize = size ? size : buf.getSizeInBytes() - offset;
    assert(actualSize <= buf.getSizeInBytes());
    assert(0 <= index && index < maxBinds);
    if (0 <= index && index < maxBinds) {
        if (auto& bind = fragmentBinds[index]) {
            // Is this the same buffer already bound to this index?
            if (bind->buf == &buf && !buf.needReBind(bind->version)) {
                // Yes, but is the offset different?
                if (bind->offset != offset) {
                    // Yes, update just the offset
                    buf.updateFragmentBindOffset(encoder, offset, index, actualSize);
                    bind->offset = offset;
                }
                return;
            }
        }
        fragmentBinds[index] = BindInfo{&buf, actualSize, offset};
    }
    buf.bindFragment(encoder, offset, index, actualSize);
}

void RenderPass::unbindFragment(std::size_t index) {
    fragmentBinds[index] = std::nullopt;
}

void RenderPass::setDepthStencilState(const MTLDepthStencilStatePtr& state) {
    if (state != currentDepthStencilState) {
        currentDepthStencilState = state;
        encoder->setDepthStencilState(currentDepthStencilState.get());
    }
}

void RenderPass::setStencilReference(int32_t referenceValue) {
    if (referenceValue != currentStencilReferenceValue) {
        currentStencilReferenceValue = referenceValue;
        encoder->setStencilReferenceValue(currentStencilReferenceValue);
    }
}

void RenderPass::setFragmentTexture(const MTLTexturePtr& texture, int32_t location) {
    assert(0 <= location && location < maxBinds);
    if (0 <= location && location < maxBinds) {
        if (fragmentTextureBindings[location] != texture) {
            fragmentTextureBindings[location] = texture;
            encoder->setFragmentTexture(texture.get(), location);
        }
    }
}

void RenderPass::setFragmentSamplerState(const MTLSamplerStatePtr& state, int32_t location) {
    assert(0 <= location && location < maxBinds);
    if (0 <= location && location < maxBinds) {
        if (fragmentSamplerStates[location] != state) {
            fragmentSamplerStates[location] = state;
            encoder->setFragmentSamplerState(state.get(), location);
        }
    }
}

/// Set the render pipeline state
void RenderPass::setRenderPipelineState(const MTLRenderPipelineStatePtr& pipelineState) {
    if (pipelineState != currentPipelineState) {
        currentPipelineState = pipelineState;
        encoder->setRenderPipelineState(currentPipelineState.get());
    }
}

void RenderPass::setCullMode(const MTL::CullMode mode) {
    if (mode != currentCullMode) {
        encoder->setCullMode(mode);
        currentCullMode = mode;
    }
}

void RenderPass::setFrontFacingWinding(const MTL::Winding winding) {
    if (winding != currentWinding) {
        encoder->setFrontFacingWinding(winding);
        currentWinding = winding;
    }
}

void RenderPass::setScissorRect(MTL::ScissorRect rect) {
    if (rect.x != currentScissorRect.x || rect.y != currentScissorRect.y || rect.width != currentScissorRect.width ||
        rect.height != currentScissorRect.height) {
        if (rect.width + rect.x > width) {
            rect.width = width - rect.x;
        }
        if (rect.height + rect.y > height) {
            rect.height = height - rect.y;
        }
        encoder->setScissorRect(rect);
        currentScissorRect = rect;
    }
}

} // namespace mtl
} // namespace mbgl
