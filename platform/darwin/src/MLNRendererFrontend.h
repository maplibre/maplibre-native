#include <mln/actor/scheduler.hpp>
#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/renderer_backend.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/renderer/renderer_frontend.hpp>
#include <mln/util/async_task.hpp>

/**
 The RenderFrontend is passed to the Map to facilitate rendering in a platform
 dependent way.
 */
class MLNRenderFrontend : public mln::RendererFrontend {
public:
  MLNRenderFrontend(std::unique_ptr<mln::Renderer> renderer_, MLNMapView* nativeView_,
                    mln::gfx::RendererBackend& mbglBackend_, bool async = false)
      : renderer(std::move(renderer_)), nativeView(nativeView_), mbglBackend(mbglBackend_) {
    if (async) {
      asyncInvalidate.emplace([&]() { [nativeView setNeedsRerender]; });
    }
  }

  void reset() override {
    if (renderer) {
      renderer.reset();
    }
  }

  void update(std::shared_ptr<mln::UpdateParameters> updateParameters_) override {
    updateParameters = std::move(updateParameters_);
    if (asyncInvalidate) {
      asyncInvalidate->send();
    } else {
      [nativeView setNeedsRerender];
    }
  }

  const mln::TaggedScheduler& getThreadPool() const override { return mbglBackend.getThreadPool(); }

  void setObserver(mln::RendererObserver& observer) override {
    if (!renderer) return;
    renderer->setObserver(&observer);
  }

  void render() {
    if (!renderer || !updateParameters) return;

    mln::gfx::BackendScope guard{mbglBackend, mln::gfx::BackendScope::ScopeType::Implicit};

    // onStyleImageMissing might be called during a render. The user implemented method
    // could trigger a call to MLNRenderFrontend#update which overwrites `updateParameters`.
    // Copy the shared pointer here so that the parameters aren't destroyed while `render(...)` is
    // still using them.
    auto updateParameters_ = updateParameters;
    renderer->render(updateParameters_);
  }

  mln::Renderer* getRenderer() { return renderer.get(); }

  void setTileCacheEnabled(bool enable) {
    if (!renderer) return;
    renderer->setTileCacheEnabled(enable);
  }

  bool getTileCacheEnabled() {
    if (!renderer) return false;
    return renderer->getTileCacheEnabled();
  }

  void reduceMemoryUse() {
    if (!renderer) return;
    renderer->reduceMemoryUse();
  }

private:
  std::unique_ptr<mln::Renderer> renderer;
  __weak MLNMapView* nativeView = nullptr;
  mln::gfx::RendererBackend& mbglBackend;
  std::shared_ptr<mln::UpdateParameters> updateParameters;
  std::optional<mln::util::AsyncTask> asyncInvalidate;
};
