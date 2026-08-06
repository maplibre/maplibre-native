#pragma once

#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/util/range.hpp>

namespace mbgl {

class AsyncRequest;
class CanonicalTileID;

namespace style {

class GeoJSONSource::Impl final : public Source::Impl {
public:
    Impl(std::string id, Immutable<GeoJSONOptions>);
    Impl(const GeoJSONSource::Impl&, std::shared_ptr<GeoJSONData>);
    ~Impl() final;

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
} // namespace mbgl
