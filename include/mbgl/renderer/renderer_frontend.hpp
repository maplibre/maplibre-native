#pragma once

#include <memory>

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/actor/scheduler.hpp>

namespace mbgl {

class RendererObserver;
class UpdateParameters;

/// The RenderFrontend is the bridge between the Map and
/// platform used to update and observe the Renderer
///
/// It hides any threading specifics and always replies on
/// the original thread.
class RendererFrontend {
public:
    virtual ~RendererFrontend() = default;

    /// Must synchronously clean up the Renderer if set
    virtual void reset() = 0;

    /// Implementer must bind the renderer observer to the renderer in a
    /// appropriate manner so that the callbacks occur on the main thread
    virtual void setObserver(RendererObserver&) = 0;

    /// Coalescing updates is up to the implementer
    virtual void update(std::shared_ptr<UpdateParameters>) = 0;

    virtual const TaggedScheduler& getThreadPool() const = 0;

protected:
};

} // namespace mbgl
