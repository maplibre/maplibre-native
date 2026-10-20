#pragma once

#include <mln/gfx/gfx_types.hpp>
#include <mln/plugin/plugin_api.h>
#include <mln/tile/geometry_tile_data.hpp>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cmath>
#include <iterator>
#include <string>
#include <vector>

namespace mln::plugin {

inline gfx::AttributeDataType attributeType(mln_plugin_vertex_attribute_type type) {
    using Type = gfx::AttributeDataType;
    constexpr Type types[] = {Type::Invalid,
                              Type::Short,
                              Type::Short2,
                              Type::UShort,
                              Type::UShort2,
                              Type::Float,
                              Type::Float2,
                              Type::Float3,
                              Type::Float4,
                              Type::UByte4Normalized};
    const auto index = static_cast<std::size_t>(type);
    return index < std::size(types) ? types[index] : Type::Invalid;
}

inline mln_plugin_geometry_type geometryType(FeatureType type) {
    switch (type) {
        case FeatureType::Point:
            return MLN_PLUGIN_GEOMETRY_POINT;
        case FeatureType::LineString:
            return MLN_PLUGIN_GEOMETRY_LINESTRING;
        case FeatureType::Polygon:
            return MLN_PLUGIN_GEOMETRY_POLYGON;
        case FeatureType::Unknown:
            return static_cast<mln_plugin_geometry_type>(0);
    }
    return static_cast<mln_plugin_geometry_type>(0);
}

inline void writePropertyJSON(rapidjson::Writer<rapidjson::StringBuffer>& writer, const Value& value) {
    value.match([&](const NullValue&) { writer.Null(); },
                [&](bool v) { writer.Bool(v); },
                [&](uint64_t v) { writer.Uint64(v); },
                [&](int64_t v) { writer.Int64(v); },
                [&](double v) { std::isfinite(v) ? writer.Double(v) : writer.Null(); },
                [&](const std::string& v) { writer.String(v.data(), static_cast<rapidjson::SizeType>(v.size())); },
                [&](const std::vector<Value>& array) {
                    writer.StartArray();
                    for (const auto& item : array) writePropertyJSON(writer, item);
                    writer.EndArray();
                },
                [&](const PropertyMap& object) {
                    writer.StartObject();
                    for (const auto& [key, item] : object) {
                        writer.Key(key.data(), static_cast<rapidjson::SizeType>(key.size()));
                        writePropertyJSON(writer, item);
                    }
                    writer.EndObject();
                });
}

// A feature's attributes as a JSON object, for layout callbacks.
inline std::string propertiesJSON(const PropertyMap& properties) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    for (const auto& [key, item] : properties) {
        writer.Key(key.data(), static_cast<rapidjson::SizeType>(key.size()));
        writePropertyJSON(writer, item);
    }
    writer.EndObject();
    return std::string(buffer.GetString(), buffer.GetSize());
}

// Owns the flattened storage borrowed by a layout or query callback. Do not move
// or copy it: the C view points into these vectors.
struct FeatureView {
    explicit FeatureView(const GeometryTileFeature& feature, uint64_t index = 0, bool withProperties = false) {
        const auto& geometry = feature.getGeometries();
        offsets.reserve(geometry.size() + 1);
        offsets.push_back(0);
        for (const auto& path : geometry) {
            for (const auto& point : path) points.push_back({point.x, point.y});
            offsets.push_back(static_cast<uint32_t>(points.size()));
        }
        value = {sizeof(value),
                 geometryType(feature.getType()),
                 index,
                 points.data(),
                 points.size(),
                 offsets.data(),
                 geometry.size(),
                 {}};
        if (withProperties) {
            propertiesStorage = propertiesJSON(feature.getProperties());
            value.properties_json = {propertiesStorage.data(), propertiesStorage.size()};
        }
    }
    FeatureView(const FeatureView&) = delete;
    FeatureView& operator=(const FeatureView&) = delete;

    std::vector<mln_plugin_tile_point_v1> points;
    std::vector<uint32_t> offsets;
    std::string propertiesStorage;
    mln_plugin_feature_v1 value{};
};

} // namespace mln::plugin
