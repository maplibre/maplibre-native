#include <mbgl/storage/file_source.hpp>
#include <mbgl/style/conversion/geojson.hpp>
#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/source_observer.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/sources/geojson_source_impl.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/thread_pool.hpp>
#include <mbgl/util/identity.hpp>

namespace mbgl {
namespace style {

// static
Immutable<GeoJSONOptions> GeoJSONOptions::defaultOptions() {
    static Immutable<GeoJSONOptions> options = makeMutable<GeoJSONOptions>();
    return options;
}

GeoJSONSource::GeoJSONSource(std::string id, Immutable<GeoJSONOptions> options)
    : Source(makeMutable<Impl>(std::move(id), std::move(options))),
      sequencedScheduler(Scheduler::GetSequenced()) {}

GeoJSONSource::~GeoJSONSource() = default;

const GeoJSONSource::Impl& GeoJSONSource::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

void GeoJSONSource::setURL(const std::string& url_) {
    url = url_;

    // Signal that the source description needs a reload
    if (loaded || req) {
        loaded = false;
        req.reset();
        observer->onSourceDescriptionChanged(*this);
    }
}

void GeoJSONSource::setGeoJSON(const mapbox::geojson::geojson& geoJSON) {
    setGeoJSONData(GeoJSONData::create(geoJSON, sequencedScheduler, impl().getOptions()));
}

void GeoJSONSource::setGeoJSONData(std::shared_ptr<GeoJSONData> geoJSONData) {
    req.reset();
    baseImpl = makeMutable<Impl>(impl(), std::move(geoJSONData));
    observer->onSourceChanged(*this);
}

std::optional<std::string> GeoJSONSource::getURL() const {
    return url;
}

const GeoJSONOptions& GeoJSONSource::getOptions() const {
    return *impl().getOptions();
}

void GeoJSONSource::loadDescription(FileSource& fileSource) {
    if (!url) {
        loaded = true;
        return;
    }

    if (req) {
        return;
    }

    req = fileSource.request(Resource::source(*url), [this](const Response& res) {
        if (res.error) {
            observer->onSourceError(*this, std::make_exception_ptr(std::runtime_error(res.error->message)));
        } else if (res.notModified) {
            return;
        } else if (res.noContent) {
            observer->onSourceError(*this, std::make_exception_ptr(std::runtime_error("unexpectedly empty GeoJSON")));
        } else {
            // Note: This task appears to be safe enough to schedule on the generic background queue.
            // This task does not reference other objects who's lifetimes are coupled with a map.
            Scheduler::GetBackground()->scheduleAndReplyValue(
                util::SimpleIdentity::Empty,
                /* makeImplInBackground */
                [currentImpl = baseImpl,
                 data = res.data,
                 seqScheduler{sequencedScheduler}]() -> Immutable<Source::Impl> {
                    assert(data);
                    auto& current = static_cast<const Impl&>(*currentImpl);
                    conversion::Error error;
                    std::shared_ptr<GeoJSONData> geoJSONData;
                    if (std::optional<GeoJSON> geoJSON = conversion::convertJSON<GeoJSON>(*data, error)) {
                        geoJSONData = GeoJSONData::create(*geoJSON, std::move(seqScheduler), current.getOptions());
                    } else {
                        // Create an empty GeoJSON VT object to make sure we're not
                        // infinitely waiting for tiles to load.
                        Log::Error(Event::ParseStyle, "Failed to parse GeoJSON data: " + error.message);
                    }
                    return makeMutable<Impl>(current, std::move(geoJSONData));
                },
                /* onImplReady */
                [this, self = makeWeakPtr(), capturedReqGeneration = ++requestGeneration](
                    Immutable<Source::Impl> newImpl) {
                    assert(capturedReqGeneration);
                    if (auto guard = self.lock(); self) {
                        if (capturedReqGeneration ==
                            requestGeneration) { // If a new request is being processed, ignore this impl.
                            baseImpl = std::move(newImpl);
                            loaded = true;
                            observer->onSourceLoaded(*this);
                        }
                    }
                });
        }
    });
}

bool GeoJSONSource::supportsLayerType(const mbgl::style::LayerTypeInfo* info) const {
    return mbgl::underlying_type(Tile::Kind::Geometry) == mbgl::underlying_type(info->tileKind);
}

Mutable<Source::Impl> GeoJSONSource::createMutable() const noexcept {
    return staticMutableCast<Source::Impl>(makeMutable<Impl>(impl()));
}

bool GeoJSONSource::isUpdateSynchronous() const noexcept {
    return baseImpl->isUpdateSynchronous();
}

} // namespace style
} // namespace mbgl
