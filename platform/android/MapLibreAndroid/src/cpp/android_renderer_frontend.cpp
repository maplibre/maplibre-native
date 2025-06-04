#include "android_renderer_frontend.hpp"

#include <mbgl/tile/tile_operation.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/util/async_task.hpp>
#include <mbgl/util/geojson.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/thread.hpp>

#include "android_renderer_backend.hpp"

namespace mbgl {
namespace android {

// Forwards RendererObserver signals to the given
// Delegate RendererObserver on the given RunLoop
class ForwardingRendererObserver : public RendererObserver {
public:
    ForwardingRendererObserver(util::RunLoop& mapRunLoop, RendererObserver& delegate_)
        : mailbox(std::make_shared<Mailbox>(mapRunLoop)),
          delegate(delegate_, mailbox) {}

    ~ForwardingRendererObserver() { mailbox->close(); }

    void onInvalidate() override { delegate.invoke(&RendererObserver::onInvalidate); }

    void onResourceError(std::exception_ptr err) override { delegate.invoke(&RendererObserver::onResourceError, err); }

    void onWillStartRenderingMap() override { delegate.invoke(&RendererObserver::onWillStartRenderingMap); }

    void onWillStartRenderingFrame() override { delegate.invoke(&RendererObserver::onWillStartRenderingFrame); }

    void onDidFinishRenderingFrame(RenderMode mode,
                                   bool repaintNeeded,
                                   bool placementChanged,
                                   const gfx::RenderingStats& stats) override {
        void (RendererObserver::*f)(
            RenderMode, bool, bool, const gfx::RenderingStats&) = &RendererObserver::onDidFinishRenderingFrame;
        delegate.invoke(f, mode, repaintNeeded, placementChanged, stats);
    }

    void onDidFinishRenderingMap() override { delegate.invoke(&RendererObserver::onDidFinishRenderingMap); }

    void onStyleImageMissing(const std::string& id, const StyleImageMissingCallback& done) override {
        delegate.invoke(&RendererObserver::onStyleImageMissing, id, done);
    }

    void onRemoveUnusedStyleImages(const std::vector<std::string>& ids) override {
        delegate.invoke(&RendererObserver::onRemoveUnusedStyleImages, ids);
    }

    void onPreCompileShader(mbgl::shaders::BuiltIn id,
                            mbgl::gfx::Backend::Type type,
                            const std::string& additionalDefines) override {
        delegate.invoke(&RendererObserver::onPreCompileShader, id, type, additionalDefines);
    }

    void onPostCompileShader(mbgl::shaders::BuiltIn id,
                             mbgl::gfx::Backend::Type type,
                             const std::string& additionalDefines) override {
        delegate.invoke(&RendererObserver::onPostCompileShader, id, type, additionalDefines);
    }

    void onShaderCompileFailed(mbgl::shaders::BuiltIn id,
                               mbgl::gfx::Backend::Type type,
                               const std::string& additionalDefines) override {
        delegate.invoke(&RendererObserver::onShaderCompileFailed, id, type, additionalDefines);
    }

    void onGlyphsLoaded(const mbgl::FontStack& stack, const mbgl::GlyphRange& range) override {
        delegate.invoke(&RendererObserver::onGlyphsLoaded, stack, range);
    }

    void onGlyphsError(const mbgl::FontStack& stack, const mbgl::GlyphRange& range, std::exception_ptr ex) override {
        delegate.invoke(&RendererObserver::onGlyphsError, stack, range, ex);
    }

    void onGlyphsRequested(const mbgl::FontStack& stack, const mbgl::GlyphRange& range) override {
        delegate.invoke(&RendererObserver::onGlyphsRequested, stack, range);
    }

    void onTileAction(TileOperation op, const OverscaledTileID& id, const std::string& sourceID) override {
        delegate.invoke(&RendererObserver::onTileAction, op, id, sourceID);
    }

private:
    std::shared_ptr<Mailbox> mailbox;
    ActorRef<RendererObserver> delegate;
};

AndroidRendererFrontend::AndroidRendererFrontend(MapRenderer& mapRenderer_)
    : mapRenderer(mapRenderer_),
      mapRunLoop(util::RunLoop::Get()),
      updateAsyncTask(std::make_unique<util::AsyncTask>([this]() {
          mapRenderer.update(std::move(updateParams));
          mapRenderer.requestRender();
      })) {}

AndroidRendererFrontend::~AndroidRendererFrontend() = default;

void AndroidRendererFrontend::reset() {
    mapRenderer.reset();
}

void AndroidRendererFrontend::setObserver(RendererObserver& observer) {
    assert(util::RunLoop::Get());
    // Don't call the Renderer directly, but use MapRenderer#setObserver to make sure
    // the Renderer may be re-initialised without losing the RendererObserver reference.
    mapRenderer.setObserver(std::make_unique<ForwardingRendererObserver>(*mapRunLoop, observer));
}

void AndroidRendererFrontend::update(std::shared_ptr<UpdateParameters> params) {
    MLN_TRACE_FUNC();
    updateParams = std::move(params);
    updateAsyncTask->send();
}

const TaggedScheduler& AndroidRendererFrontend::getThreadPool() const {
    return mapRenderer.getThreadPool();
}

void AndroidRendererFrontend::setTileCacheEnabled(bool enabled) {
    mapRenderer.actor().invoke(&Renderer::setTileCacheEnabled, enabled);
}

bool AndroidRendererFrontend::getTileCacheEnabled() const {
    return mapRenderer.actor().ask(&Renderer::getTileCacheEnabled).get();
}

void AndroidRendererFrontend::reduceMemoryUse() {
    mapRenderer.actor().invoke(&Renderer::reduceMemoryUse);
}

std::vector<Feature> AndroidRendererFrontend::querySourceFeatures(const std::string& sourceID,
                                                                  const SourceQueryOptions& options) const {
    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(&Renderer::querySourceFeatures, sourceID, options).get();
}

std::vector<Feature> AndroidRendererFrontend::queryRenderedFeatures(const ScreenBox& box,
                                                                    const RenderedQueryOptions& options) const {
    // Select the right overloaded method
    std::vector<Feature> (Renderer::*fn)(const ScreenBox&, const RenderedQueryOptions&)
        const = &Renderer::queryRenderedFeatures;

    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(fn, box, options).get();
}

std::vector<Feature> AndroidRendererFrontend::queryRenderedFeatures(const ScreenCoordinate& point,
                                                                    const RenderedQueryOptions& options) const {
    // Select the right overloaded method
    std::vector<Feature> (Renderer::*fn)(const ScreenCoordinate&, const RenderedQueryOptions&)
        const = &Renderer::queryRenderedFeatures;

    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(fn, point, options).get();
}

AnnotationIDs AndroidRendererFrontend::queryPointAnnotations(const ScreenBox& box) const {
    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(&Renderer::queryPointAnnotations, box).get();
}

AnnotationIDs AndroidRendererFrontend::queryShapeAnnotations(const ScreenBox& box) const {
    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(&Renderer::queryShapeAnnotations, box).get();
}

FeatureExtensionValue AndroidRendererFrontend::queryFeatureExtensions(
    const std::string& sourceID,
    const Feature& feature,
    const std::string& extension,
    const std::string& extensionField,
    const std::optional<std::map<std::string, mbgl::Value>>& args) const {
    return mapRenderer.actor()
        .ask(&Renderer::queryFeatureExtensions, sourceID, feature, extension, extensionField, args)
        .get();
}

} // namespace android
} // namespace mbgl
