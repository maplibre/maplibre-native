#include "ArrowPolylineExampleImpl.hpp"
#include <mbgl/map/camera.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/projection.hpp>
#include <mapbox/geojson.hpp>
#include <cmath>

namespace plugin::ex {

using mbgl::Event;
using mbgl::Log;
using mbgl::Projection;

namespace {
constexpr double kDegreesToRadians = M_PI / 180.0;
constexpr double kRadiansToDegrees = 180.0 / M_PI;
constexpr double kEarthRadiusMeters = 6371000.0;
} // namespace

ArrowPolylineExampleImpl::ArrowPolylineExampleImpl() {
    Log::Info(Event::General, "ArrowPolylineExampleImpl: Created");
}

ArrowPolylineExampleImpl::~ArrowPolylineExampleImpl() {
    Log::Info(Event::General, "ArrowPolylineExampleImpl: Destroyed");
}

uint64_t ArrowPolylineExampleImpl::get_ptr() const {
    auto basePtr = static_cast<mbgl::platform::XPlatformPlugin*>(const_cast<ArrowPolylineExampleImpl*>(this));
    return reinterpret_cast<uint64_t>(basePtr);
}

void ArrowPolylineExampleImpl::onLoad(mbgl::Map* map, mbgl::Renderer* renderer) {
    map_ = map;
    renderer_ = renderer;
    Log::Info(Event::General, "ArrowPolylineExampleImpl::onLoad");
}

void ArrowPolylineExampleImpl::onUnload() {
    Log::Info(Event::General, "ArrowPolylineExampleImpl::onUnload");
    remove_arrow_polyline();
    map_ = nullptr;
    renderer_ = nullptr;
}

void ArrowPolylineExampleImpl::onCameraDidChange(CameraChangeMode /*mode*/) {
    if (!map_ || !hasArrow_) {
        return;
    }

    double currentZoom = map_->getCameraOptions().zoom.value_or(0.0);
    if (std::abs(currentZoom - lastZoom_) >= kZoomThreshold) {
        lastZoom_ = currentZoom;
        updateSourceData();
    }
}

double ArrowPolylineExampleImpl::bearing(const LatLng& from, const LatLng& to) {
    double lat1 = from.latitude() * kDegreesToRadians;
    double lat2 = to.latitude() * kDegreesToRadians;
    double dLon = (to.longitude() - from.longitude()) * kDegreesToRadians;

    double y = std::sin(dLon) * std::cos(lat2);
    double x = std::cos(lat1) * std::sin(lat2) - std::sin(lat1) * std::cos(lat2) * std::cos(dLon);

    double bearingRad = std::atan2(y, x);
    double bearingDeg = bearingRad * kRadiansToDegrees;

    return std::fmod(bearingDeg + 360.0, 360.0);
}

ArrowPolylineExampleImpl::LatLng ArrowPolylineExampleImpl::coordinateAt(const LatLng& origin,
                                                                        double distance,
                                                                        double bearingDeg) {
    double lat1 = origin.latitude() * kDegreesToRadians;
    double lon1 = origin.longitude() * kDegreesToRadians;
    double bearingRad = bearingDeg * kDegreesToRadians;
    double angularDistance = distance / kEarthRadiusMeters;

    double lat2 = std::asin(std::sin(lat1) * std::cos(angularDistance) +
                            std::cos(lat1) * std::sin(angularDistance) * std::cos(bearingRad));

    double lon2 = lon1 + std::atan2(std::sin(bearingRad) * std::sin(angularDistance) * std::cos(lat1),
                                    std::cos(angularDistance) - std::sin(lat1) * std::sin(lat2));

    return LatLng(lat2 * kRadiansToDegrees, lon2 * kRadiansToDegrees);
}

std::vector<ArrowPolylineExampleImpl::LatLng> ArrowPolylineExampleImpl::makeChevronArrowHead(
    const std::vector<LatLng>& shaft, double headLength, double headAngle) {
    if (shaft.size() < 2) {
        return {};
    }

    const LatLng& tip = shaft.back();
    const LatLng& prev = shaft[shaft.size() - 2];

    double backBearing = bearing(tip, prev);
    double leftBearing = std::fmod(backBearing - headAngle + 360.0, 360.0);
    double rightBearing = std::fmod(backBearing + headAngle, 360.0);

    LatLng leftHead = coordinateAt(tip, headLength, leftBearing);
    LatLng rightHead = coordinateAt(tip, headLength, rightBearing);

    return {rightHead, tip, leftHead};
}

void ArrowPolylineExampleImpl::add_arrow_polyline(const std::vector<::GluecodiumPlugin::LatLng>& coordinates,
                                                  const ::GluecodiumPlugin::ArrowPolylineConfig& config) {
    if (!map_ || coordinates.size() < 2) {
        Log::Error(Event::General, "ArrowPolylineExampleImpl::add_arrow_polyline - Invalid state or coordinates");
        return;
    }

    currentCoordinates_.clear();
    for (const auto& coord : coordinates) {
        currentCoordinates_.emplace_back(coord.latitude, coord.longitude);
    }

    currentHeadLengthPixels_ = config.head_length;
    currentHeadAngle_ = config.head_angle;
    currentLineColor_ = config.line_color;
    currentLineWidth_ = config.line_width;
    hasArrow_ = true;
    lastZoom_ = map_->getCameraOptions().zoom.value_or(0.0);

    if (!layersAdded_) {
        addLayersAndSources();
    } else {
        updateSourceData();
    }

    Log::Info(Event::General, "ArrowPolylineExampleImpl::add_arrow_polyline - Arrow updated");
}

void ArrowPolylineExampleImpl::remove_arrow_polyline() {
    if (!map_) {
        return;
    }

    auto& style = map_->getStyle();

    std::string shaftLayerId = layerId_ + "-shaft";
    std::string shaftCasingLayerId = layerId_ + "-shaft-casing";
    std::string headLayerId = layerId_ + "-head";
    std::string headCasingLayerId = layerId_ + "-head-casing";
    std::string shaftSourceId = sourceId_ + "-shaft";
    std::string headSourceId = sourceId_ + "-head";

    if (style.getLayer(shaftLayerId)) {
        style.removeLayer(shaftLayerId);
    }
    if (style.getLayer(shaftCasingLayerId)) {
        style.removeLayer(shaftCasingLayerId);
    }
    if (style.getLayer(headLayerId)) {
        style.removeLayer(headLayerId);
    }
    if (style.getLayer(headCasingLayerId)) {
        style.removeLayer(headCasingLayerId);
    }
    if (style.getSource(shaftSourceId)) {
        style.removeSource(shaftSourceId);
    }
    if (style.getSource(headSourceId)) {
        style.removeSource(headSourceId);
    }

    hasArrow_ = false;
    layersAdded_ = false;
    currentCoordinates_.clear();
    Log::Info(Event::General, "ArrowPolylineExampleImpl::remove_arrow_polyline - Arrow removed");
}

void ArrowPolylineExampleImpl::addLayersAndSources() {
    if (!map_ || currentCoordinates_.size() < 2) {
        return;
    }

    auto& style = map_->getStyle();

    std::string shaftSourceId = sourceId_ + "-shaft";
    std::string shaftLayerId = layerId_ + "-shaft";
    std::string shaftCasingLayerId = layerId_ + "-shaft-casing";
    std::string headSourceId = sourceId_ + "-head";
    std::string headLayerId = layerId_ + "-head";
    std::string headCasingLayerId = layerId_ + "-head-casing";

    float casingWidth = static_cast<float>(currentLineWidth_) + 4.0f;

    // Create shaft source and layers
    auto shaftSource = std::make_unique<mbgl::style::GeoJSONSource>(shaftSourceId);
    style.addSource(std::move(shaftSource));

    auto shaftCasingLayer = std::make_unique<mbgl::style::LineLayer>(shaftCasingLayerId, shaftSourceId);
    shaftCasingLayer->setLineColor(mbgl::Color::black());
    shaftCasingLayer->setLineWidth(casingWidth);
    shaftCasingLayer->setLineCap(mbgl::style::LineCapType::Round);
    shaftCasingLayer->setLineJoin(mbgl::style::LineJoinType::Round);
    style.addLayer(std::move(shaftCasingLayer));

    auto shaftLayer = std::make_unique<mbgl::style::LineLayer>(shaftLayerId, shaftSourceId);
    shaftLayer->setLineColor(mbgl::Color::parse(currentLineColor_).value_or(mbgl::Color::red()));
    shaftLayer->setLineWidth(static_cast<float>(currentLineWidth_));
    shaftLayer->setLineCap(mbgl::style::LineCapType::Round);
    shaftLayer->setLineJoin(mbgl::style::LineJoinType::Round);
    style.addLayer(std::move(shaftLayer));

    // Create head source and layers
    auto headSource = std::make_unique<mbgl::style::GeoJSONSource>(headSourceId);
    style.addSource(std::move(headSource));

    auto headCasingLayer = std::make_unique<mbgl::style::LineLayer>(headCasingLayerId, headSourceId);
    headCasingLayer->setLineColor(mbgl::Color::black());
    headCasingLayer->setLineWidth(casingWidth);
    headCasingLayer->setLineCap(mbgl::style::LineCapType::Round);
    headCasingLayer->setLineJoin(mbgl::style::LineJoinType::Round);
    style.addLayer(std::move(headCasingLayer));

    auto headLayer = std::make_unique<mbgl::style::LineLayer>(headLayerId, headSourceId);
    headLayer->setLineColor(mbgl::Color::parse(currentLineColor_).value_or(mbgl::Color::red()));
    headLayer->setLineWidth(static_cast<float>(currentLineWidth_));
    headLayer->setLineCap(mbgl::style::LineCapType::Round);
    headLayer->setLineJoin(mbgl::style::LineJoinType::Round);
    style.addLayer(std::move(headLayer));

    layersAdded_ = true;

    updateSourceData();
}

void ArrowPolylineExampleImpl::updateSourceData() {
    if (!map_ || currentCoordinates_.size() < 2) {
        return;
    }

    auto& style = map_->getStyle();

    // Calculate arrow head based on current zoom
    double zoom = map_->getCameraOptions().zoom.value_or(0.0);
    const LatLng& tip = currentCoordinates_.back();
    double metersPerPixel = Projection::getMetersPerPixelAtLatitude(tip.latitude(), zoom);
    double headLengthMeters = currentHeadLengthPixels_ * metersPerPixel;

    auto arrowHead = makeChevronArrowHead(currentCoordinates_, headLengthMeters, currentHeadAngle_);

    // Build shaft GeoJSON
    mapbox::geojson::line_string shaftLine;
    for (const auto& coord : currentCoordinates_) {
        shaftLine.push_back(mapbox::geojson::point{coord.longitude(), coord.latitude()});
    }
    mapbox::geojson::geojson shaftGeoJson = mapbox::geojson::feature{shaftLine};

    // Build head GeoJSON
    mapbox::geojson::line_string headLine;
    for (const auto& coord : arrowHead) {
        headLine.push_back(mapbox::geojson::point{coord.longitude(), coord.latitude()});
    }
    mapbox::geojson::geojson headGeoJson = mapbox::geojson::feature{headLine};

    // Update shaft source
    std::string shaftSourceId = sourceId_ + "-shaft";
    if (auto* source = style.getSource(shaftSourceId)) {
        if (auto* geoJsonSource = source->as<mbgl::style::GeoJSONSource>()) {
            geoJsonSource->setGeoJSON(shaftGeoJson);
        }
    }

    // Update head source
    std::string headSourceId = sourceId_ + "-head";
    if (auto* source = style.getSource(headSourceId)) {
        if (auto* geoJsonSource = source->as<mbgl::style::GeoJSONSource>()) {
            geoJsonSource->setGeoJSON(headGeoJson);
        }
    }
}

} // namespace plugin::ex

// Gluecodium factory function
namespace GluecodiumPlugin {

std::shared_ptr<ArrowPolylineExample> ArrowPolylineExample::create() {
    return std::make_shared<::plugin::ex::ArrowPolylineExampleImpl>();
}

std::shared_ptr<MaplibrePlugin> MaplibrePlugin::create() {
    return nullptr;
}

} // namespace GluecodiumPlugin
