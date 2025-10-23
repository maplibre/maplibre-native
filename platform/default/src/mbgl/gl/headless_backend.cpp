#include <mbgl/gl/headless_backend.hpp>
#include <mbgl/gl/renderable_resource.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <cassert>
#include <stdexcept>
#include <type_traits>

namespace mbgl {
namespace gl {

class HeadlessRenderableResource final : public gl::RenderableResource {
public:
    HeadlessRenderableResource(HeadlessBackend& backend_, gl::Context& context_, Size size_)
        : backend(backend_),
          context(context_),
          color(context.createRenderbuffer<gfx::RenderbufferPixelType::RGBA>(size_)),
          depthStencil(context.createRenderbuffer<gfx::RenderbufferPixelType::DepthStencil>(size_)),
          framebuffer(context.createFramebuffer(color, depthStencil)) {}

    ~HeadlessRenderableResource() noexcept override = default;

    void bind() override {
        context.bindFramebuffer = framebuffer.framebuffer;
        context.scissorTest = {0, 0, 0, 0};
        context.viewport = {0, 0, framebuffer.size};
    }

    void swap() override {
        MLN_TRACE_FUNC();

        backend.swap();
    }

    HeadlessBackend& backend;
    gl::Context& context;
    gfx::Renderbuffer<gfx::RenderbufferPixelType::RGBA> color;
    gfx::Renderbuffer<gfx::RenderbufferPixelType::DepthStencil> depthStencil;
    gl::Framebuffer framebuffer;
};

HeadlessBackend::HeadlessBackend(const Size size_,
                                 gfx::HeadlessBackend::SwapBehaviour swapBehaviour_,
                                 const gfx::ContextMode contextMode_)
    : mbgl::gl::RendererBackend(contextMode_),
      mbgl::gfx::HeadlessBackend(size_),
      swapBehaviour(swapBehaviour_) {}

HeadlessBackend::~HeadlessBackend() {
    MLN_TRACE_FUNC();

    gfx::BackendScope guard{*this, gfx::BackendScope::ScopeType::Implicit};
    // Some implementations require active context for GL functions to work
    if (impl != nullptr && impl->glNeedsActiveContextOnDestruction()) {
        impl->activateContext();
    }
    // Explicitly reset the renderable resource
    resource.reset();
    // Explicitly reset the context so that it is destructed and cleaned up
    // before we destruct the impl object.
    context.reset();
    // Deactivate the context
    if (impl != nullptr && impl->glNeedsActiveContextOnDestruction()) {
        impl->deactivateContext();
    }
}

gl::ProcAddress HeadlessBackend::getExtensionFunctionPointer(const char* name) {
    assert(impl);
    return impl->getExtensionFunctionPointer(name);
}

void HeadlessBackend::activate() {
    MLN_TRACE_FUNC();

    active = true;

    if (!impl) {
        createImpl();
    }

    assert(impl);
    impl->activateContext();
}

void HeadlessBackend::deactivate() {
    MLN_TRACE_FUNC();

    assert(impl);
    impl->deactivateContext();
    active = false;
}

gfx::Renderable& HeadlessBackend::getDefaultRenderable() {
    MLN_TRACE_FUNC();

    if (!resource) {
        resource = std::make_unique<HeadlessRenderableResource>(*this, static_cast<gl::Context&>(getContext()), size);
    }
    return *this;
}

void HeadlessBackend::swap() {
    MLN_TRACE_FUNC();

    if (swapBehaviour == SwapBehaviour::Flush) static_cast<gl::Context&>(getContext()).finish();
}

void HeadlessBackend::updateAssumedState() {
    // no-op
}

PremultipliedImage HeadlessBackend::readStillImage() {
    MLN_TRACE_FUNC();

    return static_cast<gl::Context&>(getContext()).readFramebuffer<PremultipliedImage>(size);
}

RendererBackend* HeadlessBackend::getRendererBackend() {
    return this;
}

} // namespace gl

namespace gfx {

template <>
std::unique_ptr<gfx::HeadlessBackend> Backend::Create<gfx::Backend::Type::OpenGL>(
    const Size size, gfx::Renderable::SwapBehaviour swapBehavior, const gfx::ContextMode contextMode) {
    MLN_TRACE_FUNC();

    return std::make_unique<gl::HeadlessBackend>(size, swapBehavior, contextMode);
}

} // namespace gfx
} // namespace mbgl
