#include "BasicPluginExample.hpp"
#include <mbgl/map/camera.hpp>
#include <mbgl/util/geo.hpp>
#include <iostream>

namespace mbgl::platform {

BasicPluginExample::BasicPluginExample() {
  std::cout << "BasicPluginExample: Created" << std::endl;
}

BasicPluginExample::~BasicPluginExample() {
  std::cout << "BasicPluginExample: Destroyed" << std::endl;
}

void BasicPluginExample::onLoad(mbgl::Map* map, mbgl::Renderer* renderer) {
  map_ = map;
  renderer_ = renderer;
  std::cout << "BasicPluginExample::onLoad - Plugin loaded with map and renderer" << std::endl;
}

void BasicPluginExample::onUnload() {
  std::cout << "BasicPluginExample::onUnload - Plugin unloading" << std::endl;
  map_ = nullptr;
  renderer_ = nullptr;
}

void BasicPluginExample::onWillStartLoadingMap() {
  std::cout << "BasicPluginExample::onWillStartLoadingMap" << std::endl;
}

void BasicPluginExample::onDidFinishLoadingMap() {
  std::cout << "BasicPluginExample::onDidFinishLoadingMap" << std::endl;
}

void BasicPluginExample::onDidFailLoadingMap(MapLoadError error, const std::string& message) {
  std::cerr << "BasicPluginExample::onDidFailLoadingMap - Error: " << message << std::endl;
}

void BasicPluginExample::showSanFrancisco() {
  if (!map_) {
    std::cerr << "BasicPluginExample::showSanFrancisco - Map not available" << std::endl;
    return;
  }

  const double sfLatitude = 37.7749;
  const double sfLongitude = -122.4194;
  const double zoomLevel = 8.0;

  CameraOptions camera;
  camera.center = LatLng{sfLatitude, sfLongitude};
  camera.zoom = zoomLevel;

  std::cout << "BasicPluginExample::showSanFrancisco - Setting camera to ("
            << sfLatitude << ", " << sfLongitude << ") at zoom " << zoomLevel << std::endl;

  map_->jumpTo(camera);
}

} // namespace mbgl::platform
