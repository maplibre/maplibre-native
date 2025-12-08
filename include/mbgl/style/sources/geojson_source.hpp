#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/source.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/geojson.hpp>

#include <map>
#include <memory>
#include <utility>

namespace mbgl {

class AsyncRequest;
class Scheduler;
namespace style {

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
    using ClusterExpression = std::pair<std::shared_ptr<mbgl::style::expression::Expression>,
                                        std::shared_ptr<mbgl::style::expression::Expression>>;
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
};

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
class GeoJSONSource final : public Source {
public:
    GeoJSONSource(std::string id, Immutable<GeoJSONOptions> = GeoJSONOptions::defaultOptions());
    ~GeoJSONSource() final;

    void setURL(const std::string& url);
    void setGeoJSON(const GeoJSON&);
    void setGeoJSONData(std::shared_ptr<GeoJSONData>);

    std::optional<std::string> getURL() const;
    const GeoJSONOptions& getOptions() const;

    class Impl;
    const Impl& impl() const;

    void loadDescription(FileSource&) final;

    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;

    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

    bool isUpdateSynchronous() const noexcept;

protected:
    Mutable<Source::Impl> createMutable() const noexcept final;

private:
    std::optional<std::string> url;
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
} // namespace mbgl
