//
//  raw_bucket.cpp
//  App
//
//  Created by Malcolm Toon on 7/3/25.
//

#include <mbgl/util/geo.hpp>
#include <mbgl/plugin/plugin_layer.hpp>
#include <mbgl/plugin/plugin_layer_impl.hpp>

#include "raw_bucket.hpp"

#include <iostream>

using namespace mbgl;

RawBucket::~RawBucket() {
    
}

RawBucket::RawBucket(const BucketParameters& bucketParameters,
                     const std::vector<Immutable<style::LayerProperties>>& layers) {
    _layers = layers;
}

void geometryToLatLon(
  const GeometryCoordinate& coord,
  int tileX, int tileY, int zoom,
  double& lat, double& lon,
  int extent = 8192,
  int tileSize = 512
) {
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

void RawBucket::addFeature(const GeometryTileFeature& tileFeature,
                    const GeometryCollection& geometeryCollection,
                    const mbgl::ImagePositions& imagePositions,
                    const PatternLayerMap& patternLayerMap,
                    std::size_t size,
                    const CanonicalTileID& tileID) {
    
    std::shared_ptr<RawBucketFeature> tempFeature = std::make_shared<RawBucketFeature>();
    
    switch (tileFeature.getType()) {
        case FeatureType::Point:
            std::cout << "Adding Point" << "\n";
            tempFeature->_featureType = RawBucketFeature::FeatureType::FeatureTypePoint;
            break;
        case FeatureType::Unknown:
            std::cout << "Unknown Type Found\n";
            break;
        case FeatureType::LineString:
            std::cout << "LineString Type Found\n";
            tempFeature->_featureType = RawBucketFeature::FeatureType::FeatureTypeLine;
            break;
        case FeatureType::Polygon:
            std::cout << "Polygon Type Found\n";
            tempFeature->_featureType = RawBucketFeature::FeatureType::FeatureTypePolygon;
            break;

            
    }
    
    auto pm = tileFeature.getProperties();
    for (auto p: pm) {
        
        auto name = p.first;
        mapbox::feature::value value = p.second;
        
        if (auto iVal = value.getInt()) {
            std::cout << "Found Int: " << name << ": " << *iVal << "\n";
            tempFeature->_featureProperties[name] = std::to_string(*iVal);
        } else if (auto uIVal = value.getUint()) {
            std::cout << "Found UInt: " << name << ": " << *uIVal << "\n";
            tempFeature->_featureProperties[name] = std::to_string(*uIVal);

        } else if (auto s = value.getString()) {
            
            std::cout << "Found String: " << name << ": " << *s << "\n";
            tempFeature->_featureProperties[name] = *s;

        } else if (auto d = value.getDouble()) {
            std::cout << "Found Double: " << name << ": " << *d << "\n";
            tempFeature->_featureProperties[name] = std::to_string(*d);
        } else if (auto b = value.getBool()) {
            std::cout << "Found Bool: " << name << ": " << *b << "\n";
            tempFeature->_featureProperties[name] = std::to_string(*b);
        } else if (auto a = value.getArray()) {
            std::cout << "Found Array: " << name << ": " << *b << "\n";
            tempFeature->_featureProperties[name] = std::to_string(*b);
        }
        
        
//        DECLARE_VALUE_TYPE_ACCESOR(Array, array_type)
//        DECLARE_VALUE_TYPE_ACCESOR(Object, object_type)
        
    }
    
    LatLngBounds b(tileID);
    
    
    
    for (const auto& g : geometeryCollection) {
        // g is GeometryCoordinates
        RawBucketFeatureCoordinateCollection c;
        for (std::size_t i = 0, len = g.size(); i < len; i++) {
            const GeometryCoordinate& p1 = g[i];
            
            auto d = b.west();
            
//            auto c = project(
            
            
//            void geometryToLatLon(
//              const GeometryCoordinate& coord,
//              int tileX, int tileY, int zoom,
//              double& lat, double& lon,
//              int extent = 8192,
//              int tileSize = 512
            double lat = 0;
            double lon = 0;
            geometryToLatLon(p1, tileID.x, tileID.y, tileID.z, lat, lon);
            
            c._coordinates.push_back(RawBucketFeatureCoordinate(lat, lon));
        }
        tempFeature->_featureCoordinates.push_back(c);
        
    }
    
    for (auto l: _layers) {
        auto bi = l->baseImpl;
        auto bip = bi.get();
        auto pluginLayer = static_cast<const mbgl::style::PluginLayer::Impl *>(bip);
        if (pluginLayer != nullptr) {
            if (pluginLayer->_featureLoadedFunction != nullptr) {
                pluginLayer->_featureLoadedFunction(tempFeature);
            }

        }
  //      auto pluginLayer = std::dynamic_pointer_cast<std::shared_ptr<mbgl::style::PluginLayer::Impl>>(bi);

//        auto pluginLayer = std::dynamic_pointer_cast<mbgl::style::PluginLayer::Impl *>(l->baseImpl);
//        if (pluginLayer != nullptr) {
//            if (pluginLayer->_featureLoadedFunction != nullptr) {
//                pluginLayer->_featureLoadedFunction(tempFeature);
//            }
//        }
    }
    
    _features.push_back(tempFeature);
    
//    std::cout << "Adding Feature Type: " << tileFeature.getType() << "\n";
    
}

bool RawBucket::hasData() const {
    return true;
}

void RawBucket::upload(gfx::UploadPass&) {
    
}

float RawBucket::getQueryRadius(const RenderLayer&) const {
    return 0;
}

void RawBucket::update(const FeatureStates&, const GeometryTileLayer&, const std::string&, const ImagePositions&) {
    
}

