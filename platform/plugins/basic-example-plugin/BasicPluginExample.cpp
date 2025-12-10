#include "BasicPluginExample.hpp"
#include <mbgl/map/camera.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/logging.hpp>

namespace plugin::ex {

using mbgl::Log;
using mbgl::Event;
using mbgl::CameraOptions;
using mbgl::LatLng;

BasicPluginExample::BasicPluginExample() {
  Log::Info(Event::General, "BasicPluginExample: Created");
}

BasicPluginExample::~BasicPluginExample() {
  Log::Info(Event::General, "BasicPluginExample: Destroyed");
}

void BasicPluginExample::onLoad(mbgl::Map* map, mbgl::Renderer* renderer) {
  map_ = map;
  renderer_ = renderer;
  Log::Info(Event::General, "BasicPluginExample::onLoad");
}

void BasicPluginExample::onUnload() {
  Log::Info(Event::General, "BasicPluginExample::onUnload - Plugin unloading");
  map_ = nullptr;
  renderer_ = nullptr;
}

void BasicPluginExample::onWillStartLoadingMap() {
  Log::Info(Event::General, "BasicPluginExample::onWillStartLoadingMap");
}

void BasicPluginExample::onDidFinishLoadingMap() {
  Log::Info(Event::General, "BasicPluginExample::onDidFinishLoadingMap");
}

void BasicPluginExample::onDidFailLoadingMap(mbgl::MapLoadError, const std::string& message) {
  Log::Error(Event::General, "BasicPluginExample::onDidFailLoadingMap - Error: " + message);
}

void BasicPluginExample::showSanFrancisco() {
  if (!map_) {
    Log::Error(Event::General, "BasicPluginExample::showSanFrancisco - Map not available");
    return;
  }

  const double sfLatitude = 37.7749;
  const double sfLongitude = -122.4194;
  const double zoomLevel = 8.0;

  CameraOptions camera;
  camera.center = LatLng{sfLatitude, sfLongitude};
  camera.zoom = zoomLevel;

  Log::Info(Event::General, "BasicPluginExample::showSanFrancisco - Setting camera");

  map_->jumpTo(camera);
}

} // namespace plugin::ex
