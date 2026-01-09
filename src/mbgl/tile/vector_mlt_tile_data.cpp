#include <mapbox/feature.hpp>
#include <mbgl/tile/vector_mlt_tile_data.hpp>

#include <mbgl/util/constants.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#include <mlt/decoder.hpp>
#include <mlt/layer.hpp>

namespace mbgl {

using GeometryType = mlt::metadata::tileset::GeometryType;

VectorMLTTileFeature::VectorMLTTileFeature(std::shared_ptr<const MapLibreTile> tile_,
                                           const mlt::Layer& layer_,
                                           const mlt::Feature& feature_,
                                           std::uint32_t extent_)
    : tile(std::move(tile_)),
      layer(layer_),
      feature(feature_),
      extent(extent_) {}

FeatureType VectorMLTTileFeature::getType() const {
    switch (feature.getGeometry().type) {
        case GeometryType::POINT:
            return FeatureType::Point;
        case GeometryType::MULTIPOINT:
        case GeometryType::MULTILINESTRING:
        case GeometryType::LINESTRING:
            return FeatureType::LineString;
        case GeometryType::POLYGON:
        case GeometryType::MULTIPOLYGON:
            return FeatureType::Polygon;
        default:
            return FeatureType::Unknown;
    }
}

namespace {
struct PropertyVisitor {
    Value operator()(std::nullptr_t) const { return mapbox::feature::null_value; }
    Value operator()(bool value) const { return value; }
    Value operator()(std::int32_t value) const { return value; }
    Value operator()(std::uint32_t value) const { return value; }
    Value operator()(std::int64_t value) const { return value; }
    Value operator()(std::uint64_t value) const { return value; }
    Value operator()(float value) const { return value; }
    Value operator()(double value) const { return value; }
    Value operator()(std::string_view value) const { return std::string(value); }

    template <typename T>
    Value operator()(std::optional<T> value) const {
        return value ? operator()(*value) : NullValue();
    }
};
} // namespace

std::optional<Value> VectorMLTTileFeature::getValue(const std::string& key) const {
    if (auto prop = feature.getProperty(key, layer)) {
        return std::visit(PropertyVisitor(), *prop);
    }
    return std::nullopt;
}

const PropertyMap& VectorMLTTileFeature::getProperties() const {
    if (!properties) {
        const PropertyVisitor visitor;
        properties.emplace();
        properties->reserve(layer.getProperties().size());
        for (const auto& [key, props] : layer.getProperties()) {
            auto value = props.getProperty(feature.getIndex());
            auto prop = value ? std::visit(visitor, std::move(*value)) : mapbox::feature::null_value;
            properties->emplace(key, std::move(prop));
        }
    }
    return *properties;
}

FeatureIdentifier VectorMLTTileFeature::getID() const {
    return feature.getID();
}

namespace {
struct PointConverter {
    double scale;
    inline constexpr static GeometryCoordinate convert(double scale, const mlt::Coordinate& coord) {
        return {static_cast<std::int16_t>(std::round(coord.x * scale)),
                static_cast<std::int16_t>(std::round(coord.y * scale))};
    }
    GeometryCoordinate operator()(const mlt::Coordinate& coord) const { return convert(scale, coord); }
    GeometryCoordinates operator()(const mlt::CoordVec& coords) const {
        GeometryCoordinates result(coords.size());
        std::ranges::transform(coords, result.begin(), *this);
        return result;
    }
};
} // namespace

const GeometryCollection& VectorMLTTileFeature::getGeometries() const {
    MLN_TRACE_FUNC();

    if (!lines) {
        const auto scale = static_cast<double>(util::EXTENT) / extent;
        const auto& geometry = feature.getGeometry();
        const PointConverter convert{scale};
        switch (geometry.type) {
            case GeometryType::POINT: {
                const auto& geom = static_cast<const mlt::geometry::Point&>(geometry);
                lines = GeometryCollection{{convert(geom.getCoordinate())}};
                break;
            }
            case GeometryType::MULTIPOINT:
            case GeometryType::LINESTRING: {
                const auto& geom = static_cast<const mlt::geometry::MultiPoint&>(geometry);
                lines = GeometryCollection{convert(geom.getCoordinates())};
                break;
            }
            case GeometryType::POLYGON: {
                const auto& geom = static_cast<const mlt::geometry::Polygon&>(geometry);
                lines.emplace(geom.getRings().size());
                std::ranges::transform(geom.getRings(), lines->begin(), convert);
                if (!geometry.getTriangles().empty()) {
                    lines->setTriangles(tile, geometry.getTriangles());
                }
                break;
            }
            case GeometryType::MULTILINESTRING: {
                const auto& geom = static_cast<const mlt::geometry::MultiLineString&>(geometry);
                lines.emplace(geom.getLineStrings().size());
                std::ranges::transform(geom.getLineStrings(), lines->begin(), convert);
                break;
            }
            case GeometryType::MULTIPOLYGON: {
                const auto& geom = static_cast<const mlt::geometry::MultiPolygon&>(geometry);
                const auto& polygons = geom.getPolygons();
                using RingVec = mlt::geometry::MultiPolygon::RingVec;
                lines.emplace(std::accumulate(
                    polygons.begin(), polygons.end(), static_cast<std::size_t>(0), [](std::size_t a, const RingVec& b) {
                        return a + b.size();
                    }));
                for (const auto& poly : polygons) {
                    std::ranges::transform(poly, std::back_inserter(*lines), convert);
                }
                if (!geometry.getTriangles().empty()) {
                    lines->setTriangles(tile, geometry.getTriangles());
                }
                break;
            }
            default:
                lines.emplace();
                break;
        }
    }
    return *lines;
}

VectorMLTTileLayer::VectorMLTTileLayer(std::shared_ptr<const MapLibreTile> tile_, const mlt::Layer& layer_)
    : tile(std::move(tile_)),
      layer(layer_) {}

std::size_t VectorMLTTileLayer::featureCount() const {
    return layer.getFeatures().size();
}

std::unique_ptr<GeometryTileFeature> VectorMLTTileLayer::getFeature(std::size_t index) const {
    const auto& features = layer.getFeatures();
    const mlt::Feature* targetFeature = nullptr;
    targetFeature = &features[index];
    return std::make_unique<VectorMLTTileFeature>(tile, layer, *targetFeature, layer.getExtent());
}

std::string VectorMLTTileLayer::getName() const {
    return layer.getName();
}

VectorMLTTileData::VectorMLTTileData(std::shared_ptr<const std::string> data_)
    : data(std::move(data_)) {}

VectorMLTTileData::VectorMLTTileData(const VectorMLTTileData& other)
    : data(other.data),
      tile(other.tile) {}

std::unique_ptr<GeometryTileData> VectorMLTTileData::clone() const {
    return std::make_unique<VectorMLTTileData>(*this);
}

std::unique_ptr<GeometryTileLayer> VectorMLTTileData::getLayer(const std::string& name) const {
    MLN_TRACE_FUNC();

    if (data && !tile) {
        try {
            mlt::DataView tileData{data->data(), data->size()};
            tile = std::make_shared<MapLibreTile>(mlt::Decoder().decode(tileData));
        } catch (const std::exception& ex) {
            Log::Warning(Event::ParseTile, "MLT parse failed: " + std::string(ex.what()));
        }
        // We don't need the raw data anymore
        data.reset();
    }

    if (tile) {
        if (const auto* layer = tile->getLayer(name)) {
            return std::make_unique<VectorMLTTileLayer>(tile, *layer);
        }
    }
    return nullptr;
}

std::vector<std::string> VectorMLTTileData::layerNames() const {
    if (data && !data->empty() && !tile) {
        getLayer({});
    }
    if (tile) {
        std::vector<std::string> result(tile->getLayers().size());
        std::ranges::transform(tile->getLayers(), result.begin(), [](const auto& layer) { return layer.getName(); });
        return result;
    }
    return {};
}

} // namespace mbgl
