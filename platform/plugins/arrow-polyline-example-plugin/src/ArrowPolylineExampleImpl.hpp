#pragma once

#include <GluecodiumPlugin/ArrowPolylineExample.h>
#include <mbgl/plugin/cross_platform_plugin.hpp>
#include <mbgl/util/geo.hpp>
#include <vector>
#include <string>

namespace plugin::ex {

/// Implementation of ArrowPolylineExample plugin.
/// Inherits from both the Gluecodium-generated interface and XPlatformPlugin.
class ArrowPolylineExampleImpl
    : public ::GluecodiumPlugin::ArrowPolylineExample
    , public mbgl::platform::XPlatformPlugin {
public:
  ArrowPolylineExampleImpl();
  ~ArrowPolylineExampleImpl() override;

  // Gluecodium MaplibrePlugin interface
  uint64_t get_ptr() const override;

  // Gluecodium ArrowPolylineExample interface
  void add_arrow_polyline(
    const std::vector<::GluecodiumPlugin::LatLng>& coordinates,
    const ::GluecodiumPlugin::ArrowPolylineConfig& config
  ) override;
  void remove_arrow_polyline() override;

  // XPlatformPlugin interface
  void onLoad(mbgl::Map* map, mbgl::Renderer* renderer) override;
  void onUnload() override;

  // MapObserver overrides
  void onCameraDidChange(CameraChangeMode mode) override;

private:
  using LatLng = mbgl::LatLng;

  /// Generate arrow head coordinates from shaft
  static std::vector<LatLng> makeChevronArrowHead(
    const std::vector<LatLng>& shaft,
    double headLengthMeters,
    double headAngle
  );

  /// Calculate bearing from point1 to point2 in degrees
  static double bearing(const LatLng& from, const LatLng& to);

  /// Calculate coordinate at distance and bearing from origin
  static LatLng coordinateAt(const LatLng& origin, double distance, double bearingDeg);

  /// Add sources and layers to the map (called once)
  void addLayersAndSources();

  /// Update source data with current coordinates (called on updates)
  void updateSourceData();

  mbgl::Map* map_ = nullptr;
  mbgl::Renderer* renderer_ = nullptr;
  std::vector<LatLng> currentCoordinates_;
  std::string currentLineColor_ = "#FF0000";
  double currentHeadLengthPixels_ = 20.0;
  double currentHeadAngle_ = 30.0;
  double currentLineWidth_ = 3.0;
  double lastZoom_ = 0.0;
  std::string sourceId_ = "arrow-source";
  std::string layerId_ = "arrow-layer";
  bool hasArrow_ = false;
  bool layersAdded_ = false;

  static constexpr double kZoomThreshold = 0.05;
};

} // namespace plugin::ex
