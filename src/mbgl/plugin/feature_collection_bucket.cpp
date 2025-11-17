#include <mbgl/plugin/feature_collection_bucket.hpp>
#include <mbgl/plugin/plugin_layer.hpp>
#include <mbgl/plugin/plugin_layer_impl.hpp>
#include <mbgl/util/geo.hpp>

using namespace mbgl;

FeatureCollectionBucket::~FeatureCollectionBucket() {}

FeatureCollectionBucket::FeatureCollectionBucket(const BucketParameters& bucketParameters,
                                                 const std::vector<Immutable<style::LayerProperties>>& layers)
    : _tileID(bucketParameters.tileID) {
    _layers = layers;
    _featureCollection = std::make_shared<plugin::FeatureCollection>(bucketParameters.tileID);
}

void geometryToLatLon(const GeometryCoordinate& coord,
                      int tileX,
                      int tileY,
                      int zoom,
                      double& lat,
                      double& lon,
                      int extent = 8192,
                      int tileSize = 512) {
    double px = coord.x / double(extent);
    double py = coord.y / double(extent);

    double worldPixelX = (tileX + px) * tileSize;
    double worldPixelY = (tileY + py) * tileSize;

    double worldSize = tileSize * (1 << zoom);

    double mercX = worldPixelX / worldSize * 2.0 - 1.0;
    double mercY = 1.0 - worldPixelY / worldSize * 2.0;

    lon = mercX * 180.0;
    lat = 180.0 / M_PI * (2.0 * atan(exp(mercY * M_PI)) - M_PI / 2.0);
}

std::string toString(FeatureIdentifier& v) {
    auto ti = v.which();
    if (ti == 1) {
        return std::to_string(v.get<uint64_t>());
    } else if (ti == 2) {
        return std::to_string(v.get<int64_t>());
    } else if (ti == 3) {
        return v.get<std::string>();
    }
    return "";
}

void FeatureCollectionBucket::addFeature(const GeometryTileFeature& tileFeature,
                                         const GeometryCollection& geometeryCollection,
                                         [[maybe_unused]] const mbgl::ImagePositions& imagePositions,
                                         [[maybe_unused]] const PatternLayerMap& patternLayerMap,
                                         [[maybe_unused]] std::size_t size,
                                         const CanonicalTileID& tileID) {
    std::shared_ptr<plugin::Feature> tempFeature = std::make_shared<plugin::Feature>();

    auto featureID = tileFeature.getID();
    auto featureIDString = toString(featureID);
    tempFeature->_featureID = featureIDString;

    switch (tileFeature.getType()) {
        case FeatureType::Point:
            tempFeature->_featureType = plugin::Feature::FeatureType::FeatureTypePoint;
            break;
        case FeatureType::Unknown:
            break;
        case FeatureType::LineString:
            tempFeature->_featureType = plugin::Feature::FeatureType::FeatureTypeLine;
            break;
        case FeatureType::Polygon:
            tempFeature->_featureType = plugin::Feature::FeatureType::FeatureTypePolygon;
            break;
    }

    auto pm = tileFeature.getProperties();
    for (const auto& p : pm) {
        auto name = p.first;
        mapbox::feature::value value = p.second;

        if (auto iVal = value.getInt()) {
            tempFeature->_featureProperties[name] = std::to_string(*iVal);
        } else if (auto uIVal = value.getUint()) {
            tempFeature->_featureProperties[name] = std::to_string(*uIVal);
        } else if (auto s = value.getString()) {
            tempFeature->_featureProperties[name] = *s;
        } else if (auto d = value.getDouble()) {
            tempFeature->_featureProperties[name] = std::to_string(*d);
        } else if (auto b = value.getBool()) {
            tempFeature->_featureProperties[name] = std::to_string(*b);
            // TODO: Add array type
            //        } else if (auto a = value.getArray()) {
            //            tempFeature->_featureProperties[name] = std::to_string(*b);
        }
    }

    LatLngBounds b(tileID);

    for (const auto& g : geometeryCollection) {
        // g is GeometryCoordinates
        plugin::FeatureCoordinateCollection c;
        for (std::size_t i = 0, len = g.size(); i < len; i++) {
            const GeometryCoordinate& p1 = g[i];
            double lat = 0;
            double lon = 0;
            geometryToLatLon(p1, tileID.x, tileID.y, tileID.z, lat, lon);
            c._coordinates.push_back(plugin::FeatureCoordinate(lat, lon));
        }
        tempFeature->_featureCoordinates.push_back(c);
    }

    _featureCollection->_features.push_back(tempFeature);
}

bool FeatureCollectionBucket::hasData() const {
    return _featureCollection->_features.size() > 0;
}

void FeatureCollectionBucket::upload(gfx::UploadPass&) {
    uploaded = true;
}

float FeatureCollectionBucket::getQueryRadius(const RenderLayer&) const {
    return 0;
}

void FeatureCollectionBucket::update(const FeatureStates&,
                                     const GeometryTileLayer&,
                                     const std::string&,
                                     const ImagePositions&) {}
