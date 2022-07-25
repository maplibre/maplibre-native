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

CoordinatesCollections toCoordinatesCollections (const mbgl::Point<double> &point);
CoordinatesCollections toCoordinatesCollections (const mbgl::LineString<double> &lineString);
CoordinatesCollections toCoordinatesCollections (const mbgl::Polygon<double> &polygon);
CoordinatesCollections toCoordinatesCollections (const mbgl::MultiPoint<double> &points);
CoordinatesCollections toCoordinatesCollections (const mbgl::MultiLineString<double> &lineStrings);
CoordinatesCollections toCoordinatesCollections (const mbgl::MultiPolygon<double> &polygons);
QVariant toQVariant(const mbgl::Value &value);
QVariant toQVariant(const mbgl::FeatureIdentifier &id);
Feature toFeature(const mbgl::GeoJSONFeature &feature);

} // namespace QMapLibreGL::GeoJSON
