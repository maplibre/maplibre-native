#include <mbgl/tile/vector_mvt_tile_data.hpp>

#include <mbgl/util/constants.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#if ANDROID
#include <mlt/decoder.hpp>
#endif

namespace mbgl {

VectorMVTTileFeature::VectorMVTTileFeature(const mapbox::vector_tile::layer& layer, const protozero::data_view& view)
    : feature(view, layer) {}

FeatureType VectorMVTTileFeature::getType() const {
    switch (feature.getType()) {
        case mapbox::vector_tile::GeomType::POINT:
            return FeatureType::Point;
        case mapbox::vector_tile::GeomType::LINESTRING:
            return FeatureType::LineString;
        case mapbox::vector_tile::GeomType::POLYGON:
            return FeatureType::Polygon;
        default:
            return FeatureType::Unknown;
    }
}

std::optional<Value> VectorMVTTileFeature::getValue(const std::string& key) const {
    const auto value = feature.getValue(key);
    return value.is<NullValue>() ? std::nullopt : std::optional<Value>{std::move(value)};
}

const PropertyMap& VectorMVTTileFeature::getProperties() const {
    if (!properties) {
        properties = feature.getProperties();
    }
    return *properties;
}

FeatureIdentifier VectorMVTTileFeature::getID() const {
    return feature.getID();
}

const GeometryCollection& VectorMVTTileFeature::getGeometries() const {
    MLN_TRACE_FUNC();

    if (!lines) {
        const auto scale = static_cast<float>(util::EXTENT) / feature.getExtent();

        try {
            lines = feature.getGeometries<GeometryCollection>(scale);
        } catch (const std::runtime_error& ex) {
            Log::Error(Event::ParseTile, "Could not get geometries: " + std::string(ex.what()));
            lines = GeometryCollection();
        }

        if (feature.getVersion() < 2 && feature.getType() == mapbox::vector_tile::GeomType::POLYGON) {
            lines = fixupPolygons(*lines);
        }
    }
    return *lines;
}

VectorMVTTileLayer::VectorMVTTileLayer(std::shared_ptr<const std::string> data_, const protozero::data_view& view)
    : data(std::move(data_)),
      layer(view) {}

std::size_t VectorMVTTileLayer::featureCount() const {
    return layer.featureCount();
}

std::unique_ptr<GeometryTileFeature> VectorMVTTileLayer::getFeature(std::size_t i) const {
    return std::make_unique<VectorMVTTileFeature>(layer, layer.getFeature(i));
}

std::string VectorMVTTileLayer::getName() const {
    return layer.getName();
}

VectorMVTTileData::VectorMVTTileData(std::shared_ptr<const std::string> data_)
    : data(std::move(data_)) {}

std::unique_ptr<GeometryTileData> VectorMVTTileData::clone() const {
    return std::make_unique<VectorMVTTileData>(data);
}

std::unique_ptr<GeometryTileLayer> VectorMVTTileData::getLayer(const std::string& name) const {
    MLN_TRACE_FUNC();

    if (!parsed) {
        // We're parsing this lazily so that we can construct VectorTileData
        // objects on the main thread without incurring the overhead of parsing
        // immediately.
        layers = mapbox::vector_tile::buffer(*data).getLayers();
        parsed = true;
    }

    auto it = layers.find(name);
    if (it != layers.end()) {
        return std::make_unique<VectorMVTTileLayer>(data, it->second);
    }
    return nullptr;
}

std::vector<std::string> VectorMVTTileData::layerNames() const {
    return mapbox::vector_tile::buffer(*data).layerNames();
}

} // namespace mbgl
