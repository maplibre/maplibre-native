#include <mbgl/tile/vector_mlt_tile_data.hpp>

#include <mbgl/util/constants.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#include <mlt/decoder.hpp>
#include <mlt/metadata/tileset_protozero.hpp>

namespace mbgl {

VectorMLTTileFeature::VectorMLTTileFeature(std::shared_ptr<const MapLibreTile> tile_,
                                           const mlt::Feature& feature_,
                                           std::uint32_t extent_)
    : tile(std::move(tile_)),
      feature(feature_),
      extent(extent_) {}

FeatureType VectorMLTTileFeature::getType() const {
    using mlt::metadata::tileset::GeometryType;
    switch (feature.getGeometry().type) {
        case GeometryType::POINT:
            return FeatureType::Point;
        case GeometryType::LINESTRING:
            return FeatureType::LineString;
        case GeometryType::POLYGON:
            return FeatureType::Polygon;
        case GeometryType::MULTIPOINT:
        case GeometryType::MULTILINESTRING:
        case GeometryType::MULTIPOLYGON:
        default:
            return FeatureType::Unknown;
    }
}

namespace {
struct PropertyVisitor {
    Value operator()(nullptr_t) const { return nullptr; }
    Value operator()(bool value) const { return value; }
    Value operator()(std::uint32_t value) const { return value; }
    Value operator()(std::uint64_t value) const { return value; }
    Value operator()(float value) const { return value; }
    Value operator()(double value) const { return value; }
    Value operator()(std::string_view value) const { return std::string(value); }

    template <typename T>
    Value operator()(std::optional<T> value) const {
        return value ? operator()(*value) : nullptr;
    }
};
} // namespace

std::optional<Value> VectorMLTTileFeature::getValue(const std::string& key) const {
    if (const auto hit = feature.getProperties().find(key); hit != feature.getProperties().end()) {
        return std::visit(PropertyVisitor(), hit->second);
    }
    return std::nullopt;
}

const PropertyMap& VectorMLTTileFeature::getProperties() const {
    if (!properties) {
        properties.emplace();
        properties->reserve(feature.getProperties().size());
        for (const auto& [key, value] : feature.getProperties()) {
            properties->emplace(key, std::visit(PropertyVisitor(), value));
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
        using mlt::metadata::tileset::GeometryType;
        const auto scale = static_cast<double>(util::EXTENT) / extent;
        const auto& geometry = feature.getGeometry();
        const PointConverter convert{scale};
        switch (geometry.type) {
            case GeometryType::POINT: {
                const auto& point = static_cast<const mlt::Point&>(geometry);
                lines = GeometryCollection{{convert(point.getCoordinate())}};
                break;
            }
            case GeometryType::LINESTRING: {
                const auto& lineString = static_cast<const mlt::LineString&>(geometry);
                lines = GeometryCollection{convert(lineString.getCoordinates())};
                break;
            }
            case GeometryType::POLYGON: {
                const auto& poly = static_cast<const mlt::Polygon&>(geometry);
                lines.emplace(poly.getRings().size() + 1);
                lines->front() = convert(poly.getShell());
                std::ranges::transform(poly.getRings(), std::next(lines->begin()), convert);
                break;
            }
            case GeometryType::MULTIPOINT:
            case GeometryType::MULTILINESTRING:
            case GeometryType::MULTIPOLYGON:
            default:
                lines = GeometryCollection{};
                break;
        }
        // if (feature.getVersion() < 2 && feature.getType() == mapbox::vector_tile::GeomType::POLYGON) {
        //     lines = fixupPolygons(*lines);
        // }
    }
    return *lines;
}

VectorMLTTileLayer::VectorMLTTileLayer(std::shared_ptr<const MapLibreTile> tile_, const mlt::Layer& layer_)
    : tile(std::move(tile_)),
      layer(layer_) {}

std::size_t VectorMLTTileLayer::featureCount() const {
    return layer.getFeatures().size();
}

std::unique_ptr<GeometryTileFeature> VectorMLTTileLayer::getFeature(std::size_t i) const {
    return std::make_unique<VectorMLTTileFeature>(tile, layer.getFeatures().at(i), layer.getExtent());
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

    // We're parsing this lazily so that we can construct VectorTileData objects
    // on the main thread without incurring the overhead of parsing immediately.
    if (data && !tile) {
        if (data->size() > 4) {
            // Tile blobs are:
            // - metadata size as a 4-byte int
            // - metadata
            // - tile data
            const auto metadataSize = *reinterpret_cast<const std::uint32_t*>(data->data());
            if (metadataSize + 4 < data->size()) {
                try {
                    const auto metadata = mlt::metadata::tileset::read({data->data() + 4, metadataSize});
                    if (metadata) {
                        tile = std::make_shared<MapLibreTile>(mlt::Decoder().decode(
                            {data->data() + 4 + metadataSize, data->size() - metadataSize - 4}, *metadata));
                    } else {
                        Log::Warning(Event::ParseTile, "MLT parse failed");
                    }
                } catch (const std::exception& ex) {
                    Log::Warning(Event::ParseTile, "MLT Metadata parse failed: " + std::string(ex.what()));
                }
            }
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
