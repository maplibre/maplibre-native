#pragma once

#include <mapbox/geojson.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/util/feature.hpp>

#include <QMapLibreTypes>

#include <QVariant>

#include <string>

namespace QMapLibreGeoJSON {

mbgl::Point<double> asPoint(const QMapLibre::Coordinate &coordinate);
mbgl::MultiPoint<double> asMultiPoint(const QMapLibre::Coordinates &multiPoint);
mbgl::LineString<double> asLineString(const QMapLibre::Coordinates &lineString);
mbgl::MultiLineString<double> asMultiLineString(const QMapLibre::CoordinatesCollection &multiLineString);
mbgl::Polygon<double> asPolygon(const QMapLibre::CoordinatesCollection &polygon);
mbgl::MultiPolygon<double> asMultiPolygon(const QMapLibre::CoordinatesCollections &multiPolygon);
mbgl::Value asPropertyValue(const QVariant &value);
mbgl::FeatureIdentifier asFeatureIdentifier(const QVariant &id);
mbgl::GeoJSONFeature asFeature(const QMapLibre::Feature &feature);

} // namespace QMapLibreGeoJSON
