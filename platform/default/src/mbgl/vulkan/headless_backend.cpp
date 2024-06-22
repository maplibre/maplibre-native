#include <mbgl/vulkan/headless_backend.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/gfx/backend_scope.hpp>

#include <cassert>
#include <stdexcept>
#include <type_traits>

namespace mbgl {
namespace vulkan {

class HeadlessRenderableResource final : public vulkan::RenderableResource {
public:
    HeadlessRenderableResource(HeadlessBackend& backend_, vulkan::Context& context_, Size size_)
        : backend(backend_),
          context(context_),
          color(context.createRenderbuffer<gfx::RenderbufferPixelType::RGBA>(size_)),
          depthStencil(context.createRenderbuffer<gfx::RenderbufferPixelType::DepthStencil>(size_)) {}

    ~HeadlessRenderableResource() noexcept override = default;

    void bind() override {
    }

    void swap() override { backend.swap(); }

    HeadlessBackend& backend;
    vulkan::Context& context;
    gfx::Renderbuffer<gfx::RenderbufferPixelType::RGBA> color;
    gfx::Renderbuffer<gfx::RenderbufferPixelType::DepthStencil> depthStencil;
};

HeadlessBackend::HeadlessBackend(const Size size_,
                                 gfx::HeadlessBackend::SwapBehaviour swapBehaviour_,
                                 const gfx::ContextMode contextMode_)
    : mbgl::vulkan::RendererBackend(contextMode_),
      mbgl::gfx::HeadlessBackend(size_),
      swapBehaviour(swapBehaviour_) {}

HeadlessBackend::~HeadlessBackend() {
    gfx::BackendScope guard{*this, gfx::BackendScope::ScopeType::Implicit};
    
    // Explicitly reset the renderable resource
    resource.reset();
    // Explicitly reset the context so that it is destructed and cleaned up
    // before we destruct the impl object.
    context.reset();
}

void HeadlessBackend::activate() {
    active = true;

    if (!impl) {
        createImpl();
    }

    assert(impl);
}

void HeadlessBackend::deactivate() {
    assert(impl);
    active = false;
}

gfx::Renderable& HeadlessBackend::getDefaultRenderable() {
    if (!resource) {
        resource = std::make_unique<HeadlessRenderableResource>(*this, static_cast<vulkan::Context&>(getContext()), size);
    }
    return *this;
}

void HeadlessBackend::swap() {

}

PremultipliedImage HeadlessBackend::readStillImage() {
    return PremultipliedImage();
}

RendererBackend* HeadlessBackend::getRendererBackend() {
    return this;
}

void HeadlessBackend::createImpl() {
    assert(!impl);
    impl = std::make_unique<Impl>();
}

} // namespace vulkan

namespace gfx {

template <>
std::unique_ptr<gfx::HeadlessBackend> Backend::Create<gfx::Backend::Type::Vulkan>(
    const Size size, gfx::Renderable::SwapBehaviour swapBehavior, const gfx::ContextMode contextMode) {
    return std::make_unique<vulkan::HeadlessBackend>(size, swapBehavior, contextMode);
}

} // namespace gfx
} // namespace mbgl
