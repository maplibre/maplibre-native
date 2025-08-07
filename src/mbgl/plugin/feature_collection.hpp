#pragma once

#include <mbgl/tile/tile_id.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mbgl {

namespace plugin {

class FeatureCoordinate {
public:
    FeatureCoordinate(double lat, double lon)
        : _lat(lat),
          _lon(lon) {}
    double _lat = 0;
    double _lon = 0;
    double _tileX = 0; // Tile coord
    double _tileY = 0; // Tile coord
};

// This is a list of coordinates.  Broken out into its own class because
// a raw bucket feature can have an array of these
class FeatureCoordinateCollection {
public:
    std::vector<FeatureCoordinate> _coordinates;
};

class Feature {
public:
    Feature() {};
    enum class FeatureType {
        FeatureTypeUnknown,
        FeatureTypePoint,
        FeatureTypeLine,
        FeatureTypePolygon
    };
    FeatureType _featureType = FeatureType::FeatureTypeUnknown;
    std::map<std::string, std::string> _featureProperties;
    std::vector<FeatureCoordinateCollection> _featureCoordinates;
    std::string _featureID; // Unique id from the data source for this
};

class FeatureCollection {
public:
    FeatureCollection(OverscaledTileID tileID)
        : _featureCollectionTileID(tileID) {};
    std::vector<std::shared_ptr<Feature>> _features;
    OverscaledTileID _featureCollectionTileID;
};

} // namespace plugin

} // namespace mbgl
