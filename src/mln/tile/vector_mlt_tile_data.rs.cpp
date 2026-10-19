// This is an interface-compatible file analogous to vector_mlt_tile_data.cpp
// which is conditionally compiled when the optional Rust build flag is enabled.
#include <mapbox/feature.hpp>
#include <mln/tile/vector_mlt_tile_data.hpp>

#include <mln/util/constants.hpp>
#include <mln/util/instrumentation.hpp>
#include <mln/util/logging.hpp>

#include <rustutils/mlt.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace mln {
namespace {

using RustTile = rustutils::MltTile;

rust::Slice<const std::uint8_t> asBytes(const std::string& value) {
    return {reinterpret_cast<const std::uint8_t*>(value.data()), value.size()};
}

// Rust types are opaque to C++, so they can only be held through a pointer. Take over
// the Box so that layers, features and geometries can share the decoded tile.
std::shared_ptr<const RustTile> share(rust::Box<RustTile> tile) {
    return {tile.into_raw(), [](RustTile* ptr) {
                (void)rust::Box<RustTile>::from_raw(ptr);
            }};
}

std::optional<Value> convertValue(const rustutils::MltValue& value) {
    switch (value.kind) {
        case rustutils::MltValueType::Bool:
            return value.bool_value;
        case rustutils::MltValueType::Int:
            return value.int_value;
        case rustutils::MltValueType::UInt:
            return value.uint_value;
        case rustutils::MltValueType::Double:
            return value.double_value;
        case rustutils::MltValueType::String:
            return std::string(value.string_value);
        case rustutils::MltValueType::Null:
        default:
            return std::nullopt;
    }
}

FeatureType convertType(rustutils::MltFeatureType type) {
    switch (type) {
        case rustutils::MltFeatureType::Point:
            return FeatureType::Point;
        case rustutils::MltFeatureType::LineString:
            return FeatureType::LineString;
        case rustutils::MltFeatureType::Polygon:
            return FeatureType::Polygon;
        default:
            return FeatureType::Unknown;
    }
}

std::int16_t convertCoordinate(std::int32_t value, double scale) {
    constexpr auto min = static_cast<double>(std::numeric_limits<std::int16_t>::min());
    constexpr auto max = static_cast<double>(std::numeric_limits<std::int16_t>::max());
    return static_cast<std::int16_t>(std::clamp(std::round(value * scale), min, max));
}

class VectorMLTTileFeature final : public GeometryTileFeature {
public:
    VectorMLTTileFeature(std::shared_ptr<const RustTile> tile_,
                         std::size_t layerIndex_,
                         std::size_t featureIndex_,
                         double scale_)
        : tile(std::move(tile_)),
          layerIndex(layerIndex_),
          featureIndex(featureIndex_),
          scale(scale_) {}

    FeatureType getType() const override { return convertType(tile->feature_type(layerIndex, featureIndex)); }

    std::optional<Value> getValue(const std::string& key) const override {
        const auto index = tile->property_index(layerIndex, asBytes(key));
        const auto values = tile->feature_properties(layerIndex, featureIndex);
        if (index < 0 || static_cast<std::size_t>(index) >= values.size()) {
            return std::nullopt;
        }
        return convertValue(values[static_cast<std::size_t>(index)]);
    }

    const PropertyMap& getProperties() const override {
        if (!properties) {
            const auto values = tile->feature_properties(layerIndex, featureIndex);
            properties.emplace();
            properties->reserve(values.size());
            for (std::size_t i = 0; i < values.size(); ++i) {
                // Like MVT, leave out properties that the feature doesn't have.
                if (auto value = convertValue(values[i])) {
                    properties->emplace(std::string(tile->property_name(layerIndex, i)), std::move(*value));
                }
            }
        }
        return *properties;
    }

    FeatureIdentifier getID() const override {
        const auto id = tile->feature_id(layerIndex, featureIndex);
        return id.has_id ? mapbox::feature::identifier(id.value) : mapbox::feature::null_value;
    }

    const GeometryCollection& getGeometries() const override {
        MLN_TRACE_FUNC();

        if (!lines) {
            const auto offsets = tile->feature_part_offsets(layerIndex, featureIndex);
            const auto coordinates = tile->layer_coordinates(layerIndex);
            lines.emplace();
            lines->reserve(offsets.empty() ? 0 : offsets.size() - 1);
            for (std::size_t i = 0; i + 1 < offsets.size(); ++i) {
                GeometryCoordinates& part = lines->emplace_back();
                part.reserve(offsets[i + 1] - offsets[i]);
                for (auto j = offsets[i]; j < offsets[i + 1]; ++j) {
                    part.emplace_back(convertCoordinate(coordinates[j].x, scale),
                                      convertCoordinate(coordinates[j].y, scale));
                }
            }

            // The triangles live in the decoded tile, so let the geometry keep it alive.
            const auto triangles = tile->feature_triangles(layerIndex, featureIndex);
            if (!triangles.empty()) {
                lines->setTriangles(tile, std::span<const std::uint32_t>(triangles.data(), triangles.size()));
            }
        }
        return *lines;
    }

private:
    const std::shared_ptr<const RustTile> tile;
    const std::size_t layerIndex;
    const std::size_t featureIndex;
    const double scale;

    // lazy init
    mutable std::optional<GeometryCollection> lines;
    mutable std::optional<PropertyMap> properties;
};

class VectorMLTTileLayer final : public GeometryTileLayer {
public:
    VectorMLTTileLayer(std::shared_ptr<const RustTile> tile_, std::size_t layerIndex_)
        : tile(std::move(tile_)),
          layerIndex(layerIndex_),
          scale(static_cast<double>(util::EXTENT) / tile->layer_extent(layerIndex)) {}

    std::size_t featureCount() const override { return tile->feature_count(layerIndex); }

    std::unique_ptr<GeometryTileFeature> getFeature(std::size_t index) const override {
        if (index >= featureCount()) {
            throw std::out_of_range("MLT feature index out of range");
        }
        return std::make_unique<VectorMLTTileFeature>(tile, layerIndex, index, scale);
    }

    std::string getName() const override { return std::string(tile->layer_name(layerIndex)); }

private:
    const std::shared_ptr<const RustTile> tile;
    const std::size_t layerIndex;
    const double scale;
};

} // namespace

class VectorMLTTileData::Impl {
public:
    explicit Impl(std::shared_ptr<const std::string> data_)
        : data(std::move(data_)) {}

    Impl(const Impl&) = default;

    std::unique_ptr<GeometryTileLayer> getLayer(const std::string& name) const {
        MLN_TRACE_FUNC();

        decode();
        if (!tile) {
            return nullptr;
        }
        const auto index = tile->layer_index(asBytes(name));
        if (index < 0) {
            return nullptr;
        }
        return std::make_unique<VectorMLTTileLayer>(tile, static_cast<std::size_t>(index));
    }

    std::vector<std::string> layerNames() const {
        decode();
        if (!tile) {
            return {};
        }
        std::vector<std::string> result;
        result.reserve(tile->layer_count());
        for (std::size_t i = 0; i < tile->layer_count(); ++i) {
            result.emplace_back(tile->layer_name(i));
        }
        return result;
    }

private:
    // Like the C++ decoder, decode the whole tile on first access.
    void decode() const {
        if (!data) {
            return;
        }
        try {
            tile = share(rustutils::decode_mlt_tile(asBytes(*data)));
        } catch (const std::exception& ex) {
            Log::Warning(Event::ParseTile, "MLT parse failed: " + std::string(ex.what()));
        }
        // We don't need the raw data anymore
        data.reset();
    }

    mutable std::shared_ptr<const std::string> data;
    mutable std::shared_ptr<const RustTile> tile;
};

// The Rust decoder always supports FastPFOR.
VectorMLTTileData::VectorMLTTileData(std::shared_ptr<const std::string> data, bool /*fastPFOREnabled*/)
    : impl(std::make_unique<Impl>(std::move(data))) {}

VectorMLTTileData::VectorMLTTileData(const VectorMLTTileData& other)
    : impl(other.impl ? std::make_unique<Impl>(*other.impl) : nullptr) {}

VectorMLTTileData::VectorMLTTileData(VectorMLTTileData&&) noexcept = default;
VectorMLTTileData::~VectorMLTTileData() = default;

std::unique_ptr<GeometryTileData> VectorMLTTileData::clone() const {
    return std::make_unique<VectorMLTTileData>(*this);
}

std::unique_ptr<GeometryTileLayer> VectorMLTTileData::getLayer(const std::string& name) const {
    return impl ? impl->getLayer(name) : nullptr;
}

std::vector<std::string> VectorMLTTileData::layerNames() const {
    return impl ? impl->layerNames() : std::vector<std::string>{};
}

} // namespace mln
