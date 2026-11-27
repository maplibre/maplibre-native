#pragma once

#include <mln/style/source_impl.hpp>
#include <mln/style/sources/geojson_source.hpp>
#include <mln/util/range.hpp>

namespace mln {

class AsyncRequest;
class CanonicalTileID;
class CRSWorldCoordinateHelper;

namespace style {

class GeoJSONSource::Impl final : public Source::Impl {
public:
    Impl(std::string id, Immutable<GeoJSONOptions>);
    Impl(const GeoJSONSource::Impl&, std::shared_ptr<GeoJSONData>);
    ~Impl() final;

    /// `GeoJSONData::create` in a registered projection, see `addProjection`, or in lat/lng for null. In a registered
    /// projection the data is tiled in its world square, and cluster children and leaves come back in lat/lng.
    static std::shared_ptr<GeoJSONData> createData(const GeoJSON&,
                                                   std::shared_ptr<Scheduler> sequencedScheduler,
                                                   const Immutable<GeoJSONOptions>&,
                                                   std::shared_ptr<const CRSWorldCoordinateHelper> projection);

    /// The projection `setGeoJSON` would tile data set on `source` in now, for a platform that creates the data on
    /// another thread: null in lat/lng, and before the source joins a style.
    static std::shared_ptr<const CRSWorldCoordinateHelper> registeredProjection(const GeoJSONSource& source);

    Range<uint8_t> getZoomRange() const;
    std::weak_ptr<GeoJSONData> getData() const;
    const Immutable<GeoJSONOptions>& getOptions() const { return options; }

    std::optional<std::string> getAttribution() const final;

    bool isUpdateSynchronous() const final;
    void setOverrideSynchronousUpdate(bool newOverride) const;

private:
    Immutable<GeoJSONOptions> options;
    std::shared_ptr<GeoJSONData> data;
    mutable bool overrideSynchronousUpdate = false;
};

} // namespace style
} // namespace mln
