#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/util/instrumentation.hpp>
#include <mln/util/thread_local.hpp>

#include <cassert>

namespace {

mln::util::ThreadLocal<mln::gfx::BackendScope>& currentScope() {
    static mln::util::ThreadLocal<mln::gfx::BackendScope> backendScope;

    return backendScope;
}

} // namespace

namespace mln {
namespace gfx {

BackendScope::BackendScope(RendererBackend& backend_, ScopeType scopeType_)
    : priorScope(currentScope().get()),
      nextScope(nullptr),
      backend(backend_),
      scopeType(scopeType_) {
    MLN_TRACE_FUNC();

    if (priorScope) {
        assert(priorScope->nextScope == nullptr);
        priorScope->nextScope = this;
        priorScope->deactivate();
    }

    activate();

    currentScope().set(this);
}

BackendScope::~BackendScope() {
    MLN_TRACE_FUNC();

    assert(nextScope == nullptr);
    deactivate();

    if (priorScope) {
        priorScope->activate();
        currentScope().set(priorScope);
        assert(priorScope->nextScope == this);
        priorScope->nextScope = nullptr;
    } else {
        currentScope().set(nullptr);
    }
}

void BackendScope::activate() {
    MLN_TRACE_FUNC();

    if (scopeType == ScopeType::Explicit && !(priorScope && this->backend == priorScope->backend) &&
        !(nextScope && this->backend == nextScope->backend)) {
        // Only activate when set to Explicit and
        // only once per RenderBackend
        backend.activate();
        activated = true;
    }
}

void BackendScope::deactivate() {
    if (activated && !(nextScope && this->backend == nextScope->backend)) {
        // Only deactivate when set to Explicit and
        // only once per RenderBackend
        backend.deactivate();
        activated = false;
    }
}

bool BackendScope::exists() {
    return currentScope().get();
}

} // namespace gfx
} // namespace mln
