#include <mbgl/vulkan/headless_backend.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/gfx/backend_scope.hpp>

#include <cassert>
#include <stdexcept>
#include <type_traits>

namespace mbgl {
namespace vulkan {

class HeadlessRenderableResource final : public vulkan::SurfaceRenderableResource {
public:
    HeadlessRenderableResource(HeadlessBackend& backend_)
        : SurfaceRenderableResource(backend_) {
        if (backend_.getDevice() && !renderPass) {
            init(backend_.getSize().width, backend_.getSize().height);
        }
    }

    ~HeadlessRenderableResource() noexcept override = default;

    void createPlatformSurface() override {}
    void bind() override {}

    void swap() override {
        SurfaceRenderableResource::swap();
        static_cast<Context&>(backend.getContext()).waitFrame();
    }
};

HeadlessBackend::HeadlessBackend(const Size size_,
                                 gfx::HeadlessBackend::SwapBehaviour,
                                 const gfx::ContextMode contextMode_)
    : mbgl::vulkan::RendererBackend(contextMode_),
      mbgl::gfx::HeadlessBackend(size_) {
    init();
}

HeadlessBackend::~HeadlessBackend() {
    gfx::BackendScope guard{*this, gfx::BackendScope::ScopeType::Implicit};

    texture.reset();

    // Explicitly reset the renderable resource
    resource.reset();
    // Explicitly reset the context so that it is destructed and cleaned up
    // before we destruct the impl object.
    getThreadPool().runRenderJobs(true /* closeQueue */);
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
        resource = std::make_unique<HeadlessRenderableResource>(*this);
    }
    return *this;
}

PremultipliedImage HeadlessBackend::readStillImage() {
    auto& contextImpl = static_cast<Context&>(*context);
    auto& resourceImpl = static_cast<HeadlessRenderableResource&>(*resource);

    if (!texture) {
        texture = std::make_unique<Texture2D>(contextImpl);
        texture->setFormat(gfx::TexturePixelType::RGBA, gfx::TextureChannelDataType::UnsignedByte);
        texture->setSize(size);
        texture->setUsage(Texture2DUsage::Read);
    }

    contextImpl.waitFrame();
    texture->copyImage(resourceImpl.getAcquiredImage());

    return std::move(*texture->readImage());
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
