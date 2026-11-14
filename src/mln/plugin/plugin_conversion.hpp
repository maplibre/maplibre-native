#pragma once

#include <mln/gfx/gfx_types.hpp>
#include <mln/plugin/plugin_api.h>
#include <mln/tile/geometry_tile_data.hpp>

#include <iterator>
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

// The size in bytes of a property value in a shader binding's encoding, or 0 for an unknown encoding.
inline uint32_t propertyEncodingSize(mln_plugin_property_encoding_v1 encoding) {
    switch (encoding) {
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT:
        case MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT:
        case MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT:
            return 4;
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2:
            return 8;
        case MLN_PLUGIN_PROPERTY_ENCODING_IMAGE_FROM:
        case MLN_PLUGIN_PROPERTY_ENCODING_IMAGE_TO:
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR:
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR_RGBA8:
            return 16;
    }
    return 0;
}

// The vertex attribute type of a property binding. A packed binding has the minimum and the maximum in one attribute.
inline mln_plugin_vertex_attribute_type propertyAttributeType(mln_plugin_property_encoding_v1 encoding, bool packed) {
    if (packed) return propertyEncodingSize(encoding) == 4 ? MLN_PLUGIN_VERTEX_FLOAT_X2 : MLN_PLUGIN_VERTEX_FLOAT_X4;
    switch (encoding) {
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT:
        case MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT:
        case MLN_PLUGIN_PROPERTY_ENCODING_ENUM_FLOAT:
            return MLN_PLUGIN_VERTEX_FLOAT;
        case MLN_PLUGIN_PROPERTY_ENCODING_FLOAT2:
            return MLN_PLUGIN_VERTEX_FLOAT_X2;
        case MLN_PLUGIN_PROPERTY_ENCODING_IMAGE_FROM:
        case MLN_PLUGIN_PROPERTY_ENCODING_IMAGE_TO:
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR:
            return MLN_PLUGIN_VERTEX_FLOAT_X4;
        case MLN_PLUGIN_PROPERTY_ENCODING_COLOR_RGBA8:
            return MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED;
    }
    return static_cast<mln_plugin_vertex_attribute_type>(0);
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

// Owns the flattened storage borrowed by a layout or query callback. Do not move
// or copy it: the C view points into these vectors.
struct FeatureView {
    explicit FeatureView(const GeometryTileFeature& feature, uint64_t index = 0) {
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
                 geometry.size()};
    }
    FeatureView(const FeatureView&) = delete;
    FeatureView& operator=(const FeatureView&) = delete;

    std::vector<mln_plugin_tile_point_v1> points;
    std::vector<uint32_t> offsets;
    mln_plugin_feature_v1 value{};
};

} // namespace mln::plugin
