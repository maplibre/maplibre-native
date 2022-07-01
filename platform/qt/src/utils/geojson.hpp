#pragma once

#include <QMapLibreGL/Types>

#include <mapbox/geojson.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/util/feature.hpp>

#include <QVariant>

#include <string>

namespace QMapLibreGL::GeoJSON {

mbgl::Point<double> asPoint(const Coordinate &coordinate);
mbgl::MultiPoint<double> asMultiPoint(const Coordinates &multiPoint);
mbgl::LineString<double> asLineString(const Coordinates &lineString);
mbgl::MultiLineString<double> asMultiLineString(const CoordinatesCollection &multiLineString);
mbgl::Polygon<double> asPolygon(const CoordinatesCollection &polygon);
mbgl::MultiPolygon<double> asMultiPolygon(const CoordinatesCollections &multiPolygon);
mbgl::Value asPropertyValue(const QVariant &value);
mbgl::FeatureIdentifier asFeatureIdentifier(const QVariant &id);
mbgl::GeoJSONFeature asFeature(const Feature &feature);

} // namespace QMapLibreGL::GeoJSON
