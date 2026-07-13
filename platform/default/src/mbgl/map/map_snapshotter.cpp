#include <mbgl/map/map_snapshotter.hpp>

#include <mbgl/actor/actor_ref.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/map/transform.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/event.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/thread.hpp>
#include <utility>

namespace mbgl {

// static
MapSnapshotterObserver& MapSnapshotterObserver::nullObserver() {
    static MapSnapshotterObserver mapSnapshotterObserver;
    return mapSnapshotterObserver;
}

class ForwardingRendererObserver final : public RendererObserver {
public:
    explicit ForwardingRendererObserver(RendererObserver& delegate_)
        : mailbox(std::make_shared<Mailbox>(*Scheduler::GetCurrent())),
          delegate(delegate_, mailbox) {}

    ~ForwardingRendererObserver() override { mailbox->close(); }

    void onInvalidate() override { delegate.invoke(&RendererObserver::onInvalidate); }

    void onResourceError(std::exception_ptr err) override { delegate.invoke(&RendererObserver::onResourceError, err); }

    void onDidFinishRenderingFrame(RenderMode mode,
                                   bool repaintNeeded,
                                   bool placementChanged,
                                   const gfx::RenderingStats& stats) override {
        void (RendererObserver::*f)(
            RenderMode, bool, bool, const gfx::RenderingStats&) = &RendererObserver::onDidFinishRenderingFrame;
        delegate.invoke(f, mode, repaintNeeded, placementChanged, stats);
    }

    void onStyleImageMissing(const std::string& image, const StyleImageMissingCallback& cb) override {
        delegate.invoke(&RendererObserver::onStyleImageMissing, image, cb);
    }

private:
    std::shared_ptr<Mailbox> mailbox;
    ActorRef<RendererObserver> delegate;
};

class SnapshotterRenderer final : public RendererObserver {
public:
    SnapshotterRenderer(Size size, float pixelRatio, const std::optional<std::string>& localFontFamily)
        : frontend(size,
                   pixelRatio,
                   gfx::HeadlessBackend::SwapBehaviour::NoFlush,
                   gfx::ContextMode::Unique,
                   localFontFamily) {}

    void reset() {
        hasPendingStillImageRequest = false;
        frontend.reset();
    }

    void onInvalidate() override { rendererObserver->onInvalidate(); }

    void onResourceError(std::exception_ptr err) override {
        hasPendingStillImageRequest = false;
        rendererObserver->onResourceError(err);
    }

    void onDidFinishRenderingFrame(RenderMode mode, bool repaintNeeded, bool placementChanged) override {
        if (mode == RenderMode::Full && hasPendingStillImageRequest) {
            stillImage = frontend.readStillImage();
        }
        rendererObserver->onDidFinishRenderingFrame(mode, repaintNeeded, placementChanged);
    }

    void onDidFinishRenderingFrame(RenderMode mode,
                                   bool repaintNeeded,
                                   bool placementChanged,
                                   double frameEncodingTime,
                                   double frameRenderingTime) override {
        if (mode == RenderMode::Full && hasPendingStillImageRequest) {
            stillImage = frontend.readStillImage();
        }
        rendererObserver->onDidFinishRenderingFrame(
            mode, repaintNeeded, placementChanged, frameEncodingTime, frameRenderingTime);
    }

    void onDidFinishRenderingFrame(RenderMode mode,
                                   bool repaint,
                                   bool placementChanged,
                                   const gfx::RenderingStats& stats) override {
        if (mode == RenderMode::Full && hasPendingStillImageRequest) {
            stillImage = frontend.readStillImage();
        }
        rendererObserver->onDidFinishRenderingFrame(mode, repaint, placementChanged, stats);
    }

    void onStyleImageMissing(const std::string& id, const StyleImageMissingCallback& done) override {
        rendererObserver->onStyleImageMissing(id, done);
    }

    void setObserver(std::shared_ptr<RendererObserver> observer) {
        assert(observer);
        rendererObserver = std::move(observer);
        frontend.setObserver(*this);
    }

    void update(std::shared_ptr<UpdateParameters> params) {
        assert(params);
        hasPendingStillImageRequest = params->stillImageRequest;
        frontend.update(std::move(params));
    }

    void setSize(Size size) { frontend.setSize(size); }

    PremultipliedImage takeImage() {
        assert(stillImage.valid());
        return std::move(stillImage);
    }

    const TaggedScheduler& getThreadPool() const { return frontend.getThreadPool(); }

private:
    PremultipliedImage stillImage;
    bool hasPendingStillImageRequest = false;
    std::shared_ptr<RendererObserver> rendererObserver;
    HeadlessFrontend frontend;
};

class SnapshotterRendererFrontend final : public RendererFrontend {
public:
    SnapshotterRendererFrontend(Size size, float pixelRatio, std::optional<std::string> localFontFamily)
        : renderer(std::make_unique<util::Thread<SnapshotterRenderer>>(
              "Snapshotter", size, pixelRatio, std::move(localFontFamily))) {}

    ~SnapshotterRendererFrontend() override = default;

    void reset() override { renderer->actor().invoke(&SnapshotterRenderer::reset); }

    void setObserver(RendererObserver& observer) override {
        renderer->actor().invoke(&SnapshotterRenderer::setObserver,
                                 std::make_unique<ForwardingRendererObserver>(observer));
    }

    void update(std::shared_ptr<UpdateParameters> parameters) override {
        updateParameters = std::move(parameters);
        renderer->actor().invoke(&SnapshotterRenderer::update, updateParameters);
    }

    void setSize(Size size) { renderer->actor().invoke(&SnapshotterRenderer::setSize, size); }

    const TransformState& getTransformState() const {
        assert(updateParameters);
        static TransformState defaultTransformState{};
        if (updateParameters) return updateParameters->transformState;
        return defaultTransformState;
    }

    PremultipliedImage takeImage() { return renderer->actor().ask(&SnapshotterRenderer::takeImage).get(); }

    const mbgl::TaggedScheduler& getThreadPool() const override {
        return renderer->actor().ask(&SnapshotterRenderer::getThreadPool).get();
    }

private:
    std::shared_ptr<UpdateParameters> updateParameters;
    const std::unique_ptr<util::Thread<SnapshotterRenderer>> renderer;
};

class MapSnapshotter::Impl final : public MapObserver {
public:
    Impl(Size size,
         float pixelRatio,
         const ResourceOptions& resourceOptions,
         const ClientOptions& clientOptions,
         MapSnapshotterObserver& observer_,
         std::optional<std::string> localFontFamily)
        : observer(observer_),
          frontend(size, pixelRatio, std::move(localFontFamily)),
          map(frontend,
              *this,
              MapOptions().withMapMode(MapMode::Static).withSize(size).withPixelRatio(pixelRatio),
              resourceOptions,
              clientOptions),
          region(LatLngBounds::empty()),
          regionInsets{0, 0, 0, 0} {}

    void setRegion(const LatLngBounds& _region) {
        region = _region;
        std::vector<LatLng> latLngs = {region.southwest(), region.northeast()};
        map.jumpTo(map.cameraForLatLngs(latLngs, regionInsets));
    }

    void setPadding(const mbgl::EdgeInsets& insets) {
        regionInsets = insets;
        if (!region.isEmpty()) {
            std::vector<LatLng> latLngs = {region.southwest(), region.northeast()};
            map.jumpTo(map.cameraForLatLngs(latLngs, regionInsets));
        }
    }

    mbgl::EdgeInsets getPadding() const { return regionInsets; }

    void addAnnotationImage(std::unique_ptr<style::Image> image) { map.addAnnotationImage(std::move(image)); }

    void addAnnotation(const Annotation& annotation) { map.addAnnotation(annotation); }

    void snapshot(MapSnapshotter::Callback callback) {
        if (!callback) {
            Log::Error(Event::General, "MapSnapshotter::Callback is not set");
            return;
        }

        if (renderStillCallback) {
            callback(std::make_exception_ptr(util::MisuseException("MapSnapshotter is currently rendering an image")),
                     PremultipliedImage(),
                     {},
                     {},
                     {});
        }

        renderStillCallback = std::make_unique<Actor<MapSnapshotter::Callback>>(
            *Scheduler::GetCurrent(),
            [this, cb = std::move(callback)](std::exception_ptr ptr,
                                             PremultipliedImage image,
                                             Attributions attributions,
                                             PointForFn pfn,
                                             LatLngForFn latLonFn) {
                cb(std::move(ptr), std::move(image), std::move(attributions), std::move(pfn), std::move(latLonFn));
                renderStillCallback.reset();
            });

        map.renderStill([this, actorRef = renderStillCallback->self()](const std::exception_ptr& error) {
            // Create lambda that captures the current transform state
            // and can be used to translate for geographic to screen
            // coordinates
            assert(map.getCameraOptions().center);
            PointForFn pointForFn = [center = *map.getCameraOptions().center,
                                     transformState = frontend.getTransformState()](const LatLng& latLng) {
                LatLng unwrappedLatLng = latLng.wrapped();
                unwrappedLatLng.unwrapForShortestPath(center);
                Transform transform{transformState};
                return transform.latLngToScreenCoordinate(unwrappedLatLng);
            };

            // Create lambda that captures the current transform state
            // and can be used to translate for geographic to screen
            // coordinates
            LatLngForFn latLngForFn =
                [transformState = frontend.getTransformState()](const ScreenCoordinate& screenCoordinate) {
                    Transform transform{transformState};
                    return transform.screenCoordinateToLatLng(screenCoordinate);
                };

            // Collect all source attributions
            std::vector<std::string> attributions;
            for (auto source : map.getStyle().getSources()) {
                auto attribution = source->getAttribution();
                if (attribution) {
                    attributions.push_back(*attribution);
                }
            }

            // Invoke callback
            actorRef.invoke(&MapSnapshotter::Callback::operator(),
                            error,
                            error ? PremultipliedImage() : frontend.takeImage(),
                            std::move(attributions),
                            std::move(pointForFn),
                            std::move(latLngForFn));
        });
    }

    // MapObserver overrides
    void onDidFailLoadingMap(MapLoadError, const std::string& error) override { observer.onDidFailLoadingStyle(error); }
    void onDidFinishLoadingStyle() override { observer.onDidFinishLoadingStyle(); }
    void onStyleImageMissing(const std::string& image) override { observer.onStyleImageMissing(image); }

    Map& getMap() { return map; }
    const Map& getMap() const { return map; }
    SnapshotterRendererFrontend& getRenderer() { return frontend; }
    void cancel() { renderStillCallback.reset(); }

private:
    std::unique_ptr<Actor<MapSnapshotter::Callback>> renderStillCallback;
    MapSnapshotterObserver& observer;
    SnapshotterRendererFrontend frontend;
    Map map;
    LatLngBounds region;
    mbgl::EdgeInsets regionInsets;
};

MapSnapshotter::MapSnapshotter(Size size,
                               float pixelRatio,
                               const ResourceOptions& resourceOptions,
                               const ClientOptions& clientOptions,
                               MapSnapshotterObserver& observer,
                               std::optional<std::string> localFontFamily)
    : impl(std::make_unique<MapSnapshotter::Impl>(
          size, pixelRatio, resourceOptions, clientOptions, observer, std::move(localFontFamily))) {}

MapSnapshotter::MapSnapshotter(Size size,
                               float pixelRatio,
                               const ResourceOptions& resourceOptions,
                               const ClientOptions& clientOptions)
    : MapSnapshotter(size, pixelRatio, resourceOptions, clientOptions, MapSnapshotterObserver::nullObserver()) {}

MapSnapshotter::~MapSnapshotter() = default;

void MapSnapshotter::setStyleURL(const std::string& styleURL) {
    impl->getMap().getStyle().loadURL(styleURL);
}

std::string MapSnapshotter::getStyleURL() const {
    return impl->getMap().getStyle().getURL();
}

void MapSnapshotter::setStyleJSON(const std::string& styleJSON) {
    impl->getMap().getStyle().loadJSON(styleJSON);
}

std::string MapSnapshotter::getStyleJSON() const {
    return impl->getMap().getStyle().getJSON();
}

void MapSnapshotter::setSize(const Size& size) {
    impl->getMap().setSize(size);
    impl->getRenderer().setSize(size);
}

Size MapSnapshotter::getSize() const {
    return impl->getMap().getMapOptions().size();
}

void MapSnapshotter::setCameraOptions(const CameraOptions& options) {
    CameraOptions optionsWithPadding = options;
    if (!options.padding) {
        // Apply region padding if no padding is explicitly set in camera options
        optionsWithPadding.withPadding(impl->getPadding());
    }
    impl->getMap().jumpTo(optionsWithPadding);
}

CameraOptions MapSnapshotter::getCameraOptions() const {
    return impl->getMap().getCameraOptions();
}

void MapSnapshotter::setRegion(const LatLngBounds& region) {
    impl->setRegion(region);
}

LatLngBounds MapSnapshotter::getRegion() const {
    return impl->getMap().latLngBoundsForCamera(impl->getMap().getCameraOptions(getPadding()));
}

void MapSnapshotter::setPadding(const mbgl::EdgeInsets& insets) {
    impl->setPadding(insets);
}

mbgl::EdgeInsets MapSnapshotter::getPadding() const {
    return impl->getPadding();
}

void MapSnapshotter::addAnnotationImage(std::unique_ptr<style::Image> image) {
    impl->addAnnotationImage(std::move(image));
}

void MapSnapshotter::addAnnotation(const Annotation& annotation) {
    impl->addAnnotation(annotation);
}

style::Style& MapSnapshotter::getStyle() {
    return impl->getMap().getStyle();
}

const style::Style& MapSnapshotter::getStyle() const {
    return impl->getMap().getStyle();
}

void MapSnapshotter::snapshot(MapSnapshotter::Callback callback) {
    impl->snapshot(std::move(callback));
}

void MapSnapshotter::cancel() {
    impl->cancel();
}

} // namespace mbgl
