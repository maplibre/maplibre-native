#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/style/projection.hpp>
#include <mln/style/source.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/geojson.hpp>

#include <map>
#include <memory>
#include <utility>

namespace mln {

class AsyncRequest;
class Scheduler;
namespace style {

class Style;

struct GeoJSONOptions {
    // GeoJSON-VT options
    uint8_t minzoom = 0;
    uint8_t maxzoom = 18;
    uint16_t tileSize = util::tileSize_I;
    uint16_t buffer = 128;
    double tolerance = 0.375;
    bool lineMetrics = false;

    // Supercluster options
    bool cluster = false;
    uint16_t clusterRadius = 50;
    uint8_t clusterMaxZoom = 17;
    size_t clusterMinPoints = 2;
    using ClusterExpression = std::pair<std::shared_ptr<mln::style::expression::Expression>,
                                        std::shared_ptr<mln::style::expression::Expression>>;
    using ClusterProperties = std::map<std::string, ClusterExpression>;
    ClusterProperties clusterProperties;

    // Update options
    bool synchronousUpdate = false;

    static Immutable<GeoJSONOptions> defaultOptions();
};
class GeoJSONData {
public:
    using TileFeatures = mapbox::feature::feature_collection<int16_t>;
    using Features = mapbox::feature::feature_collection<double>;
    static std::shared_ptr<GeoJSONData> create(const GeoJSON&,
                                               std::shared_ptr<Scheduler> sequencedScheduler,
                                               const Immutable<GeoJSONOptions>& = GeoJSONOptions::defaultOptions());

    virtual ~GeoJSONData() = default;
    virtual void getTile(const CanonicalTileID&, const std::function<void(TileFeatures)>&, bool runSynchronously) = 0;

    // SuperclusterData
    virtual Features getChildren(std::uint32_t) = 0;
    virtual Features getLeaves(std::uint32_t, std::uint32_t limit, std::uint32_t offset) = 0;
    virtual std::uint8_t getClusterExpansionZoom(std::uint32_t) = 0;

    /// The name of the registered projection the data was tiled in, or empty.
    const std::string& getProjectionName() const { return projectionName; }

private:
    friend class GeoJSONSource;
    std::string projectionName;
};

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
class GeoJSONSource final : public Source {
public:
    GeoJSONSource(std::string id, Immutable<GeoJSONOptions> = GeoJSONOptions::defaultOptions());
    ~GeoJSONSource() final;

    void setURL(const std::string& url);
    void setGeoJSON(const GeoJSON&);
    void setGeoJSON(GeoJSON&&);
    /// Sets data that is already tiled. It draws only in the projection it was tiled in, so data from
    /// `GeoJSONData::create`, which tiles in Mercator, draws nothing in a registered projection and logs a warning.
    void setGeoJSONData(std::shared_ptr<GeoJSONData>);

    std::optional<std::string> getURL() const;
    const GeoJSONOptions& getOptions() const;

    class Impl;
    const Impl& impl() const;

    void loadDescription(FileSource&) final;

    bool supportsLayerType(const mln::style::LayerTypeInfo*) const override;

    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

    bool isUpdateSynchronous() const noexcept;
    void setOverrideSynchronousUpdate(bool) const noexcept;

protected:
    Mutable<Source::Impl> createMutable() const noexcept final;

private:
    friend class Style;
    /// Tiles the data in the projection of the style the source is in. The style calls it when the source joins it and
    /// when its projection changes. The source then tiles the GeoJSON it keeps, requests its `url` again, or warns that
    /// its data, tiled for another projection, draws nothing.
    void setProjection(Immutable<Projection::Impl>);
    /// Whether GeoJSON set now is kept: until the source joins a style, and while its style is in a registered
    /// projection.
    bool keepsGeoJSON() const;
    /// Tiles `keptGeoJSON` in the projection of the style the source is in.
    void tileKeptGeoJSON();

    std::optional<std::string> url;
    /// The projection of the style the source is in, empty until it joins one.
    std::optional<Immutable<Projection::Impl>> projection;
    /// The GeoJSON set on the source, kept to tile it when the source joins a style and, while the style is in a
    /// registered projection, to tile it again when the projection changes.
    std::unique_ptr<GeoJSON> keptGeoJSON;
    std::unique_ptr<AsyncRequest> req;
    std::atomic<uint64_t> requestGeneration{0};
    std::shared_ptr<Scheduler> sequencedScheduler;
    mapbox::base::WeakPtrFactory<Source> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`
};

template <>
inline bool Source::is<GeoJSONSource>() const {
    return getType() == SourceType::GeoJSON;
}

} // namespace style
} // namespace mln
