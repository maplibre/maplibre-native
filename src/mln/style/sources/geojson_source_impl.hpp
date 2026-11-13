#pragma once

#include <mln/style/source_impl.hpp>
#include <mln/style/sources/geojson_source.hpp>
#include <mln/util/range.hpp>

namespace mln {

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
    void setUpdateSynchronous(bool value) { synchronousUpdate = value; }

private:
    Immutable<GeoJSONOptions> options;
    std::shared_ptr<GeoJSONData> data;
    bool synchronousUpdate;
};

} // namespace style
} // namespace mln
