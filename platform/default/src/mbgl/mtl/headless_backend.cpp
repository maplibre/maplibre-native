#include <mbgl/mtl/headless_backend.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/offscreen_texture.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/gfx/backend_scope.hpp>

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <Metal/Metal.hpp>

namespace mbgl {
namespace mtl {

class HeadlessRenderableResource final : public mtl::RenderableResource {
public:
    HeadlessRenderableResource(HeadlessBackend& backend_, mtl::Context& context_, Size size)
        : backend(backend_),
          context(context_) {
        offscreenTexture = context.createOffscreenTexture(size, gfx::TextureChannelDataType::UnsignedByte);
    }

    ~HeadlessRenderableResource() noexcept override = default;

    void bind() override {
        offscreenTexture->getResource<RenderableResource>().bind();
    }

    void swap() override {
        if (backend.getSwapBehaviour() == HeadlessBackend::SwapBehaviour::Flush) {
            offscreenTexture->getResource<RenderableResource>().swap();
        }
    }
    
    PremultipliedImage readStillImage() {
        return offscreenTexture->readStillImage();
    }
    
    const RendererBackend& getBackend() const override { return context.getBackend(); }

    const MTLCommandBufferPtr& getCommandBuffer() const override {
        return offscreenTexture->getResource<RenderableResource>().getCommandBuffer();
    }

    MTLBlitPassDescriptorPtr getUploadPassDescriptor() const override {
        return offscreenTexture->getResource<RenderableResource>().getUploadPassDescriptor();
    }

    MTLRenderPassDescriptorPtr getRenderPassDescriptor() const override {
        return offscreenTexture->getResource<RenderableResource>().getRenderPassDescriptor();
    }

    HeadlessBackend& backend;
    Context& context;
    std::unique_ptr<gfx::OffscreenTexture> offscreenTexture;
};

HeadlessBackend::HeadlessBackend(const Size size_,
                                 gfx::HeadlessBackend::SwapBehaviour swapBehaviour_,
                                 const gfx::ContextMode contextMode_)
    : mbgl::mtl::RendererBackend(contextMode_),
      mbgl::gfx::HeadlessBackend(size_),
      swapBehaviour(swapBehaviour_) {}

HeadlessBackend::~HeadlessBackend() {
    gfx::BackendScope guard{*this, gfx::BackendScope::ScopeType::Implicit};
    // Explicitly reset the renderable resource
    resource.reset();
    // Explicitly reset the context
    context.reset();
}

void HeadlessBackend::activate() {
    active = true;
}

void HeadlessBackend::deactivate() {
    active = false;
}

gfx::Renderable& HeadlessBackend::getDefaultRenderable() {
    if (!resource) {
        resource = std::make_unique<HeadlessRenderableResource>(*this, static_cast<mtl::Context&>(getContext()), size);
    }
    return *this;
}

void HeadlessBackend::updateAssumedState() {
    // no-op
}

PremultipliedImage HeadlessBackend::readStillImage() {
    return getResource<HeadlessRenderableResource>().readStillImage();
}

RendererBackend* HeadlessBackend::getRendererBackend() {
    return this;
}

HeadlessBackend::SwapBehaviour HeadlessBackend::getSwapBehaviour() {
    return swapBehaviour;
}

} // namespace mtl

namespace gfx {

template <>
std::unique_ptr<gfx::HeadlessBackend> Backend::Create<gfx::Backend::Type::Metal>(
    const Size size, gfx::HeadlessBackend::SwapBehaviour swapBehavior, const gfx::ContextMode contextMode) {
    return std::make_unique<mtl::HeadlessBackend>(size, swapBehavior, contextMode);
}

} // namespace gfx
} // namespace mbgl
