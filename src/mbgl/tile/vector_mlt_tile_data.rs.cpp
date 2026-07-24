#include <mapbox/feature.hpp>
#include <mbgl/tile/vector_mlt_tile_data.hpp>

#include <mbgl/util/constants.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#include <rustutils/mlt.hpp>

#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace mbgl {
namespace {

std::string toStdString(rust::String value) {
    return {value.data(), value.size()};
}

rust::Str toRustStr(const std::string& value) {
    return {value.data(), value.size()};
}

struct RustMLTTile {
    explicit RustMLTTile(rust::Box<rustutils::MltTile> tile_)
        : tile(std::move(tile_)) {}

    rust::Box<rustutils::MltTile> tile;
};

template <class StringGetter>
std::optional<Value> convertValue(const rustutils::MltValue& value, StringGetter&& getString) {
    switch (value.typ) {
        case rustutils::MltValueType::Missing:
            return std::nullopt;
        case rustutils::MltValueType::Null:
            return mapbox::feature::null_value;
        case rustutils::MltValueType::Bool:
            return value.bool_value;
        case rustutils::MltValueType::I64:
            return value.i64_value;
        case rustutils::MltValueType::U64:
            return value.u64_value;
        case rustutils::MltValueType::F32:
            return static_cast<double>(value.f32_value);
        case rustutils::MltValueType::F64:
            return value.f64_value;
        case rustutils::MltValueType::String:
            return toStdString(getString());
        default:
            return std::nullopt;
    }
}

FeatureType featureType(rustutils::MltGeometryType type) {
    switch (type) {
        case rustutils::MltGeometryType::Point:
            return FeatureType::Point;
        case rustutils::MltGeometryType::MultiPoint:
        case rustutils::MltGeometryType::LineString:
        case rustutils::MltGeometryType::MultiLineString:
            return FeatureType::LineString;
        case rustutils::MltGeometryType::Polygon:
        case rustutils::MltGeometryType::MultiPolygon:
            return FeatureType::Polygon;
        default:
            return FeatureType::Unknown;
    }
}

bool hasTriangles(rustutils::MltGeometryType type) {
    return type == rustutils::MltGeometryType::Polygon || type == rustutils::MltGeometryType::MultiPolygon;
}

GeometryCoordinate convertCoordinate(double scale, rustutils::MltCoordinate coord) {
    return {static_cast<std::int16_t>(std::round(static_cast<double>(coord.x) * scale)),
            static_cast<std::int16_t>(std::round(static_cast<double>(coord.y) * scale))};
}

class VectorMLTTileFeature final : public GeometryTileFeature {
public:
    VectorMLTTileFeature(std::shared_ptr<RustMLTTile> tile_,
                         std::size_t layerIndex_,
                         std::size_t featureIndex_,
                         std::uint32_t extent_,
                         rustutils::MltGeometryType geometryType_,
                         std::shared_ptr<const std::vector<std::string>> propertyNames_)
        : tile(std::move(tile_)),
          layerIndex(layerIndex_),
          featureIndex(featureIndex_),
          extent(extent_),
          geometryType(geometryType_),
          propertyNames(std::move(propertyNames_)) {}

    VectorMLTTileFeature(const VectorMLTTileFeature&) = delete;
    VectorMLTTileFeature(VectorMLTTileFeature&& other)
        : tile(std::move(other.tile)),
          layerIndex(other.layerIndex),
          featureIndex(other.featureIndex),
          extent(other.extent),
          geometryType(other.geometryType),
          propertyNames(std::move(other.propertyNames)),
          lines(std::move(other.lines)),
          properties(std::move(other.properties)) {}

    VectorMLTTileFeature& operator=(VectorMLTTileFeature&&) = delete;
    VectorMLTTileFeature& operator=(const VectorMLTTileFeature&) = delete;

    FeatureType getType() const override { return featureType(geometryType); }

    std::optional<Value> getValue(const std::string& key) const override {
        const auto value = tile->tile->property_value_by_name(layerIndex, featureIndex, toRustStr(key));
        return convertValue(
            value, [&] { return tile->tile->property_string_value_by_name(layerIndex, featureIndex, toRustStr(key)); });
    }

    const PropertyMap& getProperties() const override {
        if (!properties) {
            properties.emplace();
            const auto count = propertyNames ? propertyNames->size() : tile->tile->property_count(layerIndex);
            const auto values = tile->tile->property_values(layerIndex, featureIndex);
            properties->reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                auto name = propertyNames ? (*propertyNames)[i] : toStdString(tile->tile->property_name(layerIndex, i));
                const auto value = i < values.size() ? values[i]
                                                     : tile->tile->property_value(layerIndex, featureIndex, i);
                auto converted = convertValue(
                    value, [&] { return tile->tile->property_string_value(layerIndex, featureIndex, i); });
                properties->emplace(std::move(name), converted.value_or(mapbox::feature::null_value));
            }
        }
        return *properties;
    }

    FeatureIdentifier getID() const override {
        const auto id = tile->tile->feature_id(layerIndex, featureIndex);
        return id.has_id ? mapbox::feature::identifier(id.value) : mapbox::feature::null_value;
    }

    const GeometryCollection& getGeometries() const override {
        MLN_TRACE_FUNC();

        if (!lines) {
            const auto scale = static_cast<double>(util::EXTENT) / extent;
            const auto offsets = tile->tile->geometry_part_offsets(layerIndex, featureIndex);
            const auto rustCoordinates = tile->tile->geometry_coordinates(layerIndex, featureIndex);
            lines.emplace();
            lines->reserve(offsets.size() > 0 ? offsets.size() - 1 : 0);

            for (std::size_t partIndex = 0; partIndex + 1 < offsets.size(); ++partIndex) {
                const auto start = static_cast<std::size_t>(offsets[partIndex]);
                const auto end = static_cast<std::size_t>(offsets[partIndex + 1]);
                GeometryCoordinates coordinates;
                coordinates.reserve(end - start);
                for (std::size_t coordinateIndex = start; coordinateIndex < end; ++coordinateIndex) {
                    coordinates.push_back(convertCoordinate(scale, rustCoordinates[coordinateIndex]));
                }
                lines->push_back(std::move(coordinates));
            }

            const auto rustTriangles = tile->tile->geometry_triangles(layerIndex, featureIndex);
            if (hasTriangles(geometryType) && rustTriangles.size() > 0) {
                auto triangles = std::make_shared<std::vector<std::uint32_t>>();
                triangles->reserve(rustTriangles.size());
                for (std::size_t i = 0; i < rustTriangles.size(); ++i) {
                    triangles->push_back(rustTriangles[i]);
                }
                lines->setTriangles(triangles, std::span<const std::uint32_t>(triangles->data(), triangles->size()));
            }
        }

        return *lines;
    }

private:
    std::shared_ptr<RustMLTTile> tile;
    std::size_t layerIndex;
    std::size_t featureIndex;
    std::uint32_t extent;
    rustutils::MltGeometryType geometryType;
    std::shared_ptr<const std::vector<std::string>> propertyNames;

    mutable std::optional<GeometryCollection> lines;
    mutable std::optional<PropertyMap> properties;
};

class VectorMLTTileLayer final : public GeometryTileLayer {
public:
    VectorMLTTileLayer(std::shared_ptr<RustMLTTile> tile_, std::size_t layerIndex_)
        : tile(std::move(tile_)),
          layerIndex(layerIndex_),
          featureCountValue(tile->tile->feature_count(layerIndex)),
          extent(tile->tile->layer_extent(layerIndex)),
          propertyNames(loadPropertyNames()) {}

    std::size_t featureCount() const override { return featureCountValue; }

    std::unique_ptr<GeometryTileFeature> getFeature(std::size_t index) const override {
        if (index >= featureCountValue) {
            throw std::out_of_range("MLT feature index out of range");
        }
        return std::make_unique<VectorMLTTileFeature>(
            tile, layerIndex, index, extent, tile->tile->feature_type(layerIndex, index), propertyNames);
    }

    std::string getName() const override { return toStdString(tile->tile->layer_name(layerIndex)); }

private:
    std::shared_ptr<const std::vector<std::string>> loadPropertyNames() const {
        auto names = std::make_shared<std::vector<std::string>>();
        const auto count = tile->tile->property_count(layerIndex);
        names->reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            names->push_back(toStdString(tile->tile->property_name(layerIndex, i)));
        }
        return names;
    }

    std::shared_ptr<RustMLTTile> tile;
    std::size_t layerIndex;
    std::size_t featureCountValue;
    std::uint32_t extent;
    std::shared_ptr<const std::vector<std::string>> propertyNames;
};

} // namespace

class VectorMLTTileData::Impl {
public:
    explicit Impl(std::shared_ptr<const std::string> data_)
        : data(std::move(data_)) {}

    std::unique_ptr<GeometryTileLayer> getLayer(const std::string& name) const {
        MLN_TRACE_FUNC();

        loadFull();
        if (!tile) {
            return nullptr;
        }

        const auto index = tile->tile->layer_index_by_name(toRustStr(name));
        if (index < 0) {
            return nullptr;
        }

        return std::make_unique<VectorMLTTileLayer>(tile, static_cast<std::size_t>(index));
    }

    std::vector<std::string> layerNames() const {
        loadMetadata();
        if (!tile) {
            return {};
        }

        std::vector<std::string> result;
        const auto count = tile->tile->layer_count();
        result.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            result.push_back(toStdString(tile->tile->layer_name(i)));
        }
        return result;
    }

private:
    void loadMetadata() const {
        if (!data || tile) {
            return;
        }

        try {
            const auto bytes = reinterpret_cast<const std::uint8_t*>(data->data());
            auto decoded = rustutils::parse_mlt_tile_metadata(rust::Slice<const std::uint8_t>(bytes, data->size()));
            tile = std::make_shared<RustMLTTile>(std::move(decoded));
        } catch (const std::exception& ex) {
            Log::Warning(Event::ParseTile, "MLT parse failed: " + std::string(ex.what()));
            data.reset();
        }
    }

    void loadFull() const {
        if (tile && tile->tile->is_decoded()) {
            return;
        }
        if (!data) {
            tile.reset();
            return;
        }

        try {
            const auto bytes = reinterpret_cast<const std::uint8_t*>(data->data());
            auto decoded = rustutils::decode_mlt_tile(rust::Slice<const std::uint8_t>(bytes, data->size()));
            tile = std::make_shared<RustMLTTile>(std::move(decoded));
        } catch (const std::exception& ex) {
            Log::Warning(Event::ParseTile, "MLT parse failed: " + std::string(ex.what()));
            tile.reset();
        }
        data.reset();
    }

    mutable std::shared_ptr<const std::string> data;
    mutable std::shared_ptr<RustMLTTile> tile;
};

VectorMLTTileData::VectorMLTTileData(std::shared_ptr<const std::string> data, bool)
    : impl(std::make_shared<Impl>(std::move(data))) {}

VectorMLTTileData::VectorMLTTileData(const VectorMLTTileData&) = default;
VectorMLTTileData::VectorMLTTileData(VectorMLTTileData&&) noexcept = default;
VectorMLTTileData::~VectorMLTTileData() = default;

std::unique_ptr<GeometryTileData> VectorMLTTileData::clone() const {
    return std::make_unique<VectorMLTTileData>(*this);
}

std::unique_ptr<GeometryTileLayer> VectorMLTTileData::getLayer(const std::string& name) const {
    return impl->getLayer(name);
}

std::vector<std::string> VectorMLTTileData::layerNames() const {
    return impl->layerNames();
}

} // namespace mbgl
