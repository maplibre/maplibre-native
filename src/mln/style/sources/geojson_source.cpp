#include <mln/map/world_coordinate_helper.hpp>
#include <mln/storage/file_source.hpp>
#include <mln/style/conversion/geojson.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/projection_impl.hpp>
#include <mln/style/source_observer.hpp>
#include <mln/style/sources/geojson_source.hpp>
#include <mln/style/sources/geojson_source_impl.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/async_request.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/thread_pool.hpp>
#include <mln/util/identity.hpp>

#include <utility>

namespace mln {
namespace style {

namespace {

/// Whether the data is tiled where a style in the registered projection draws it, or in lat/lng for null.
bool isTiledIn(const GeoJSONData& data, const CRSWorldCoordinateHelper* projection) {
    return projection ? data.getProjection() == projection->name() : data.getProjection().empty();
}

void warnThatItDrawsNothing(const std::string& id) {
    Log::Warning(Event::General,
                 "GeoJSON source \"" + id +
                     "\" has data tiled for another projection and draws nothing; set its data again after changing "
                     "the projection.");
}

} // namespace

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

/*!
 * \brief GeoJSONSource::setURL
 * Set the url to a geojson file which can be fetched
 * \param url_
 */
void GeoJSONSource::setURL(const std::string& url_) {
    url = url_;
    keptGeoJSON.reset();

    // Signal that the source description needs a reload
    if (loaded || req) {
        loaded = false;
        req.reset();
        observer->onSourceDescriptionChanged(*this);
    }
}

void GeoJSONSource::setGeoJSON(const mapbox::geojson::geojson& geoJSON) {
    if (keepsGeoJSON()) {
        setGeoJSON(GeoJSON(geoJSON));
        return;
    }
    setGeoJSONData(GeoJSONData::create(geoJSON, sequencedScheduler, impl().getOptions()));
}

void GeoJSONSource::setGeoJSON(mapbox::geojson::geojson&& geoJSON) {
    if (!keepsGeoJSON()) {
        setGeoJSON(std::as_const(geoJSON));
        return;
    }
    req.reset();
    keptGeoJSON = std::make_unique<GeoJSON>(std::move(geoJSON));
    if (projection) {
        tileKeptGeoJSON();
        observer->onSourceChanged(*this);
    }
}

void GeoJSONSource::setGeoJSONData(std::shared_ptr<GeoJSONData> geoJSONData) {
    keptGeoJSON.reset();
    req.reset();
    if (projection && geoJSONData && !isTiledIn(*geoJSONData, (*projection)->registeredProjection.get())) {
        warnThatItDrawsNothing(impl().id);
    }
    baseImpl = makeMutable<Impl>(impl(), std::move(geoJSONData));
    observer->onSourceChanged(*this);
}

void GeoJSONSource::setProjection(Immutable<Projection::Impl> projection_) {
    const auto previous = std::exchange(projection, std::move(projection_));
    const CRSWorldCoordinateHelper* registered = (*projection)->registeredProjection.get();
    if (previous && (*previous)->registeredProjection.get() == registered) {
        return;
    }
    if (keptGeoJSON) {
        tileKeptGeoJSON();
        if (!registered) {
            keptGeoJSON.reset();
        }
        observer->onSourceChanged(*this);
    } else if (req) {
        req.reset();
        ++requestGeneration;
        loaded = false;
        observer->onSourceDescriptionChanged(*this);
    } else if (const auto data = impl().getData().lock();
               data && !isTiledIn(*data, registered) &&
               (!previous || isTiledIn(*data, (*previous)->registeredProjection.get()))) {
        warnThatItDrawsNothing(impl().id);
    }
}

bool GeoJSONSource::keepsGeoJSON() const {
    return !projection || (*projection)->registeredProjection != nullptr;
}

void GeoJSONSource::tileKeptGeoJSON() {
    baseImpl = makeMutable<Impl>(
        impl(),
        Impl::createData(*keptGeoJSON, sequencedScheduler, impl().getOptions(), Impl::registeredProjection(*this)));
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
                 seqScheduler{sequencedScheduler},
                 registeredProjection = projection ? (*projection)->registeredProjection
                                                   : nullptr]() -> Immutable<Source::Impl> {
                    assert(data);
                    auto& current = static_cast<const Impl&>(*currentImpl);
                    conversion::Error error;
                    std::shared_ptr<GeoJSONData> geoJSONData;
                    if (std::optional<GeoJSON> geoJSON = conversion::convertJSON<GeoJSON>(*data, error)) {
                        geoJSONData = Impl::createData(
                            *geoJSON, std::move(seqScheduler), current.getOptions(), registeredProjection);
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

bool GeoJSONSource::supportsLayerType(const mln::style::LayerTypeInfo* info) const {
    return mln::underlying_type(Tile::Kind::Geometry) == mln::underlying_type(info->tileKind);
}

Mutable<Source::Impl> GeoJSONSource::createMutable() const noexcept {
    return staticMutableCast<Source::Impl>(makeMutable<Impl>(impl()));
}

bool GeoJSONSource::isUpdateSynchronous() const noexcept {
    return baseImpl->isUpdateSynchronous();
}

void GeoJSONSource::setOverrideSynchronousUpdate(bool value) const noexcept {
    impl().setOverrideSynchronousUpdate(value);
}

} // namespace style
} // namespace mln
