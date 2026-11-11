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
