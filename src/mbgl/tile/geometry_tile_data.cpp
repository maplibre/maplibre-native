#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/math/clamp.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/math.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4005)
#endif

#include <mapbox/geometry/wagyu/wagyu.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <numbers>

using namespace std::numbers;

namespace mbgl {
namespace {

double signedArea(const GeometryCoordinates& ring) {
    double sum = 0;

    for (std::size_t i = 0, len = ring.size(), j = len - 1; i < len; j = i++) {
        const GeometryCoordinate& p1 = ring[i];
        const GeometryCoordinate& p2 = ring[j];
        sum += (p2.x - p1.x) * (p1.y + p2.y);
    }

    return sum;
}

LinearRing<int32_t> toWagyuPath(const GeometryCoordinates& ring) {
    MLN_TRACE_FUNC();

    LinearRing<int32_t> result;
    result.reserve(ring.size());
    for (const auto& p : ring) {
        result.emplace_back(p.x, p.y);
    }
    return result;
}

GeometryCollection toGeometryCollection(MultiPolygon<int16_t>&& multipolygon) {
    MLN_TRACE_FUNC();

    GeometryCollection result;
    for (auto& polygon : multipolygon) {
        for (auto& ring : polygon) {
            result.emplace_back(std::move(ring));
        }
    }
    return result;
}
} // namespace

GeometryCollection fixupPolygons(const GeometryCollection& rings) {
    MLN_TRACE_FUNC();

    using namespace mapbox::geometry::wagyu;

    wagyu<int32_t> clipper;

    for (const auto& ring : rings) {
        clipper.add_ring(toWagyuPath(ring));
    }

    MultiPolygon<int16_t> multipolygon;
    clipper.execute(clip_type_union, multipolygon, fill_type_even_odd, fill_type_even_odd);

    return toGeometryCollection(std::move(multipolygon));
}

std::vector<GeometryCollection> classifyRings(const GeometryCollection& rings) {
    MLN_TRACE_FUNC();

    std::vector<GeometryCollection> polygons;

    std::size_t len = rings.size();

    if (len <= 1) {
        polygons.emplace_back(rings.clone());
        return polygons;
    }

    GeometryCollection polygon;
    int8_t ccw = 0;

    for (const auto& ring : rings) {
        double area = signedArea(ring);
        if (area == 0) continue;

        if (ccw == 0) {
            ccw = (area < 0 ? -1 : 1);
        }

        if (ccw == (area < 0 ? -1 : 1) && !polygon.empty()) {
            polygons.emplace_back(std::move(polygon));
            polygon = GeometryCollection();
        }

        polygon.emplace_back(ring);
    }

    if (!polygon.empty()) {
        polygons.emplace_back(std::move(polygon));
    }

    return polygons;
}

void limitHoles(GeometryCollection& polygon, uint32_t maxHoles) {
    MLN_TRACE_FUNC();

    if (polygon.size() > 1 + maxHoles) {
        std::nth_element(
            polygon.begin() + 1, polygon.begin() + 1 + maxHoles, polygon.end(), [](const auto& a, const auto& b) {
                return std::fabs(signedArea(a)) > std::fabs(signedArea(b));
            });
        polygon.resize(1 + maxHoles);
    }
}

Feature::geometry_type convertGeometry(const GeometryTileFeature& geometryTileFeature, const CanonicalTileID& tileID) {
    MLN_TRACE_FUNC();

    const double size = util::EXTENT * std::pow(2, tileID.z);
    const double x0 = util::EXTENT * static_cast<double>(tileID.x);
    const double y0 = util::EXTENT * static_cast<double>(tileID.y);

    auto tileCoordinatesToLatLng = [&](const Point<int16_t>& p) {
        double y2 = 180 - (p.y + y0) * 360 / size;
        return Point<double>((p.x + x0) * 360 / size - 180, std::atan(std::exp(y2 * pi / 180)) * 360.0 / pi - 90.0);
    };

    const GeometryCollection& geometries = geometryTileFeature.getGeometries();

    switch (geometryTileFeature.getType()) {
        case FeatureType::Unknown: {
            assert(false);
            return Point<double>(NAN, NAN);
        }

        case FeatureType::Point: {
            MultiPoint<double> multiPoint;
            for (const auto& p : geometries.at(0)) {
                multiPoint.push_back(tileCoordinatesToLatLng(p));
            }
            if (multiPoint.size() == 1) {
                return multiPoint[0];
            } else {
                return multiPoint;
            }
        }

        case FeatureType::LineString: {
            MultiLineString<double> multiLineString;
            for (const auto& g : geometries) {
                LineString<double> lineString;
                for (const auto& p : g) {
                    lineString.push_back(tileCoordinatesToLatLng(p));
                }
                multiLineString.push_back(std::move(lineString));
            }
            if (multiLineString.size() == 1) {
                return multiLineString[0];
            } else {
                return multiLineString;
            }
        }

        case FeatureType::Polygon: {
            MultiPolygon<double> multiPolygon;
            for (const auto& pg : classifyRings(geometries)) {
                Polygon<double> polygon;
                for (const auto& r : pg) {
                    LinearRing<double> linearRing;
                    for (const auto& p : r) {
                        linearRing.push_back(tileCoordinatesToLatLng(p));
                    }
                    polygon.push_back(std::move(linearRing));
                }
                multiPolygon.push_back(std::move(polygon));
            }
            if (multiPolygon.size() == 1) {
                return multiPolygon[0];
            } else {
                return multiPolygon;
            }
        }
    }

    // Unreachable, but placate GCC.
    return Point<double>();
}

GeometryCollection convertGeometry(const Feature::geometry_type& geometryTileFeature, const CanonicalTileID& tileID) {
    MLN_TRACE_FUNC();

    const double size = util::EXTENT * std::pow(2, tileID.z);
    const double x0 = util::EXTENT * static_cast<double>(tileID.x);
    const double y0 = util::EXTENT * static_cast<double>(tileID.y);

    auto latLonToTileCoodinates = [&](const Point<double>& c) {
        Point<int16_t> p;

        auto x = (c.x + 180.0) * size / 360.0 - x0;
        p.x = int16_t(util::clamp<int64_t>(
            static_cast<int16_t>(x), std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max()));

        auto y = (180 - (std::log(std::tan((c.y + 90) * pi / 360.0)) * 180 / pi)) * size / 360 - y0;
        p.y = int16_t(util::clamp<int64_t>(
            static_cast<int16_t>(y), std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max()));

        return p;
    };

    return geometryTileFeature.match(
        [&](const Point<double>& point) -> GeometryCollection { return {{latLonToTileCoodinates(point)}}; },
        [&](const MultiPoint<double>& points) -> GeometryCollection {
            MultiPoint<int16_t> result;
            result.reserve(points.size());
            for (const auto& p : points) {
                result.emplace_back(latLonToTileCoodinates(p));
            }
            return {std::move(result)};
        },
        [&](const LineString<double>& lineString) -> GeometryCollection {
            LineString<int16_t> result;
            result.reserve(lineString.size());
            for (const auto& p : lineString) {
                result.emplace_back(latLonToTileCoodinates(p));
            }
            return {std::move(result)};
        },
        [&](const MultiLineString<double>& lineStrings) -> GeometryCollection {
            GeometryCollection result;
            result.reserve(lineStrings.size());
            for (const auto& line : lineStrings) {
                LineString<int16_t> temp;
                temp.reserve(line.size());
                for (const auto& p : line) {
                    temp.emplace_back(latLonToTileCoodinates(p));
                }
                result.emplace_back(temp);
            }
            return result;
        },
        [&](const Polygon<double>& polygon) -> GeometryCollection {
            GeometryCollection result;
            result.reserve(polygon.size());
            for (const auto& ring : polygon) {
                LinearRing<int16_t> temp;
                temp.reserve(ring.size());
                for (const auto& p : ring) {
                    temp.emplace_back(latLonToTileCoodinates(p));
                }
                result.emplace_back(temp);
            }
            return result;
        },
        [&](const MultiPolygon<double>& polygons) -> GeometryCollection {
            GeometryCollection result;
            result.reserve(polygons.size());
            for (const auto& pg : polygons) {
                for (const auto& r : pg) {
                    LinearRing<int16_t> ring;
                    ring.reserve(r.size());
                    for (const auto& p : r) {
                        ring.emplace_back(latLonToTileCoodinates(p));
                    }
                    result.emplace_back(ring);
                }
            }
            return result;
        },
        [](const auto&) -> GeometryCollection { return GeometryCollection(); });
}

Feature convertFeature(const GeometryTileFeature& geometryTileFeature, const CanonicalTileID& tileID) {
    MLN_TRACE_FUNC();

    Feature feature{convertGeometry(geometryTileFeature, tileID)};
    feature.properties = geometryTileFeature.getProperties();
    feature.id = geometryTileFeature.getID();
    return feature;
}

const PropertyMap& GeometryTileFeature::getProperties() const {
    static const PropertyMap dummy;
    return dummy;
}

const GeometryCollection& GeometryTileFeature::getGeometries() const {
    static const GeometryCollection dummy;
    return dummy;
}

GeometryCollectionFloat roundPolygonCorners(GeometryCollection& polygon, double desiredCornerDistance) {
    const int arcPoints = 3;
    const double maxEdgeLenPercent = 0.2;
    const double sinParallelTreshold = sin(util::deg2rad(5));
    GeometryCollectionFloat roundedCornerPolygon;

    for (auto ring : polygon) {
        GeometryCoordinatesFloat roundedCornerRing;

        std::size_t nVertices = ring.size() - 1;
        for (std::size_t i = 0; i < nVertices; i++) {
            auto prevPoint = convertPoint<double>(ring[(i - 1 + nVertices) % nVertices]);
            auto cornerPoint = convertPoint<double>(ring[i]);
            auto nextPoint = convertPoint<double>(ring[(i + 1) % nVertices]);

            // Compute the start and end points of the rounded corner
            auto edge1Vector = util::normal<double>(prevPoint, cornerPoint);
            auto edge2Vector = util::normal<double>(cornerPoint, nextPoint);

            auto edge1MaxCornerDistance = util::dist<double>(cornerPoint, prevPoint) * maxEdgeLenPercent;
            auto edge2MaxCornerDistance = util::dist<double>(cornerPoint, nextPoint) * maxEdgeLenPercent;
            auto cornerDistance = std::min({desiredCornerDistance, edge1MaxCornerDistance, edge2MaxCornerDistance});

            auto startPoint = cornerPoint - edge1Vector * cornerDistance;
            auto endPoint = cornerPoint + edge2Vector * cornerDistance;

            // Perpendicular directions
            auto perp1Vector = util::perp(edge1Vector);
            auto perp2Vector = util::perp(edge2Vector);

            // Ensure perpendiculars point toward same side
            if (util::crossProduct(edge1Vector, edge2Vector) < 0) {
                perp1Vector *= -1.0;
                perp2Vector *= -1.0;
            }

            // Center = intersection of perpendiculars
            auto perpCrossProduct = util::crossProduct(perp1Vector, perp2Vector);
            if (std::abs(perpCrossProduct) < sinParallelTreshold) {
                roundedCornerRing.emplace_back(convertPoint<float>(ring[i]));
                continue;
            }
            auto t = util::crossProduct(endPoint - startPoint, perp2Vector) / perpCrossProduct;
            auto centerPoint = startPoint + perp1Vector * t;

            // Add the start point of the rounded corner
            roundedCornerRing.emplace_back(convertPoint<float>(startPoint));

            // Generate points along the arc
            auto radius = util::dist<double>(startPoint, centerPoint);
            auto startAngle = std::atan2(startPoint.y - centerPoint.y, startPoint.x - centerPoint.x);
            auto arcAngle = util::angle_between(startPoint - centerPoint, endPoint - centerPoint);
            for (int k = 1; k <= arcPoints; k++) {
                double angle = startAngle + arcAngle * k / (arcPoints + 1);
                auto arcPoint = Point(centerPoint.x + std::cos(angle) * radius, centerPoint.y + std::sin(angle) * radius);
                roundedCornerRing.emplace_back(convertPoint<float>(arcPoint));
            }

            // Add the end point of the rounded corner
            roundedCornerRing.emplace_back(convertPoint<float>(endPoint));
        }
        // Close the ring by repeating the first point at the end
        roundedCornerRing.emplace_back(roundedCornerRing[0]);

        // Add the ring to the rounded corner polygon
        roundedCornerPolygon.emplace_back(roundedCornerRing);
    }
    return roundedCornerPolygon;
}
} // namespace mbgl
