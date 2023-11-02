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

    if (!resource.getCommandBuffer()) {
        resource.bind();
    }

    if (const auto& buffer = resource.getCommandBuffer()) {
        if (auto rpd = resource.getRenderPassDescriptor()) {
            if (descriptor.clearColor) {
                if (auto copy = NS::TransferPtr(rpd->copy())) {
                    if (auto* colorTarget = copy->colorAttachments()->object(0)) {
                        const auto& c = *descriptor.clearColor;
                        colorTarget->setLoadAction(MTL::LoadActionClear);
                        colorTarget->setClearColor(MTL::ClearColor::Make(c.r, c.g, c.b, c.a));
                        rpd = copy;
                    }
                }
            }
            encoder = NS::RetainPtr(buffer->renderCommandEncoder(rpd.get()));
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

    commandEncoder.context.clear();
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
    assert(0 <= index && index < maxBinds);
    if (0 <= index && index < maxBinds) {
        if (auto& bind = vertexBinds[index]) {
            // Is this the same buffer already bound to this index?
            if (bind->buf == &buf) {
                // Yes, but is the offset different?
                if (bind->offset != offset) {
                    // Yes, update just the offset
                    buf.updateVertexBindOffset(encoder, offset, index, size);
                    bind->offset = offset;
                }
                return;
            }
        }
        vertexBinds[index] = BindInfo{&buf, offset};
    }
    buf.bindVertex(encoder, offset, index, size);
}

void RenderPass::bindFragment(const BufferResource& buf, std::size_t offset, std::size_t index, std::size_t size) {
    assert(0 <= index && index < maxBinds);
    if (0 <= index && index < maxBinds) {
        if (auto& bind = fragmentBinds[index]) {
            // Is this the same buffer already bound to this index?
            if (bind->buf == &buf) {
                // Yes, but is the offset different?
                if (bind->offset != offset) {
                    // Yes, update just the offset
                    buf.updateFragmentBindOffset(encoder, offset, index, size);
                    bind->offset = offset;
                }
                return;
            }
        }
        fragmentBinds[index] = BindInfo{&buf, offset};
    }
    buf.bindFragment(encoder, offset, index, size);
}

} // namespace mtl
} // namespace mbgl
