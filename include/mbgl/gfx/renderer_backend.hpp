#pragma once

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/util.hpp>

#include <memory>
#include <mutex>

namespace mbgl {

class ProgramParameters;
class Map;

namespace gfx {

class BackendScope;
class Context;
class Renderable;
class ShaderRegistry;

// We can make some optimizations if we know that the drawing context is not shared with other code.
enum class ContextMode : bool {
    Unique,
    Shared,
};

class RendererBackend {
protected:
    explicit RendererBackend(ContextMode);
    RendererBackend(ContextMode, const TaggedScheduler&);

public:
    virtual ~RendererBackend();
    RendererBackend(const RendererBackend&) = delete;
    RendererBackend& operator=(const RendererBackend&) = delete;

    // Return the background thread pool assigned to this backend
    TaggedScheduler& getThreadPool() noexcept { return threadPool; }

    /// Returns the device's context.
    Context& getContext();

    template <typename T>
    T& getContext() {
        return static_cast<T&>(getContext());
    }

    bool contextIsShared() const { return contextMode == ContextMode::Shared; }

    /// Returns a reference to the default surface that should be rendered on.
    virtual Renderable& getDefaultRenderable() = 0;

    /// One-time shader initialization
    virtual void initShaders(gfx::ShaderRegistry&, const ProgramParameters&) = 0;
    const mbgl::util::SimpleIdentity uniqueID;

protected:
    virtual std::unique_ptr<Context> createContext() = 0;

    /// Called when the backend's GL context needs to be made active or
    /// inactive. These are called, as a matched pair, exclusively through
    /// BackendScope, in two situations:
    ///
    ///   1. When releasing GL resources during Renderer destruction
    ///      (Including calling CustomLayerHost::deinitialize during
    ///      RenderCustomLayer destruction)
    ///   2. When renderering through Renderer::render()
    ///      (Including calling CustomLayerHost::initialize for newly added
    ///      custom layers and
    ///       CustomLayerHost::deinitialize on layer removal)
    virtual void activate() = 0;
    virtual void deactivate() = 0;

protected:
    std::unique_ptr<Context> context;
    const ContextMode contextMode;
    std::once_flag initialized;
    TaggedScheduler threadPool;

    friend class BackendScope;
};

constexpr bool operator==(const RendererBackend& a, const RendererBackend& b) {
    return &a == &b;
}

} // namespace gfx
} // namespace mbgl
