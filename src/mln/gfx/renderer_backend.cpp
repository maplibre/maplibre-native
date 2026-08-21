#include <mln/gfx/renderer_backend.hpp>
#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/context.hpp>

namespace mln {
namespace gfx {

RendererBackend::RendererBackend(const ContextMode contextMode_)
    : contextMode(contextMode_),
      threadPool(Scheduler::GetBackground(), uniqueID) {}

RendererBackend::RendererBackend(const ContextMode contextMode_, const TaggedScheduler& threadPool_)
    : contextMode(contextMode_),
      threadPool(threadPool_) {}

RendererBackend::~RendererBackend() = default;

gfx::Context& RendererBackend::getContext() {
    assert(BackendScope::exists());
    std::call_once(initialized, [this] { context = createContext(); });
    assert(context);
    return *context;
}

} // namespace gfx
} // namespace mln
