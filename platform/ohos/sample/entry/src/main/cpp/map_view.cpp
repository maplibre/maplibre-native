#include "map_view.hpp"

#if MLN_RENDER_BACKEND_VULKAN
#include "vulkan_window_backend.hpp"
#else
#include "egl_window_backend.hpp"
#endif

#include <mbgl/math/angles.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/style/source.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/string.hpp>

#include <algorithm>
#include <cmath>
#include <exception>
#include <set>
#include <stdexcept>
#include <utility>

namespace mbgl {
namespace ohos {
namespace {

std::uint32_t logicalDimensionForFramebufferDimension(const std::uint32_t dimension, const float pixelRatio) {
    return std::max<std::uint32_t>(1, static_cast<std::uint32_t>(std::ceil(dimension / pixelRatio)));
}

Size logicalSizeForFramebufferSize(const Size size, const float pixelRatio) {
    return {logicalDimensionForFramebufferDimension(size.width, pixelRatio),
            logicalDimensionForFramebufferDimension(size.height, pixelRatio)};
}

double logicalCoordinateForFramebufferCoordinate(const double coordinate, const float pixelRatio) {
    return coordinate / pixelRatio;
}

ScreenCoordinate logicalCoordinateForFramebufferCoordinate(const double x, const double y, const float pixelRatio) {
    return {logicalCoordinateForFramebufferCoordinate(x, pixelRatio),
            logicalCoordinateForFramebufferCoordinate(y, pixelRatio)};
}

} // namespace

MapView::MapView(const float pixelRatio_)
    : pixelRatio(pixelRatio_) {
    if (!std::isfinite(pixelRatio) || pixelRatio <= 0.0f) {
        throw std::invalid_argument("Pixel ratio must be a finite positive value");
    }
}

MapView::~MapView() {
    clearSurface();
}

void MapView::setSurface(OHNativeWindow* newWindow, Size size) {
    if (newWindow == nullptr || size.isEmpty()) {
        clearSurface();
        return;
    }

    if (backend && window == newWindow) {
        setSize(size);
        return;
    }

    createMap(newWindow, size);
}

void MapView::clearSurface() {
    map.reset();
    frontend.reset();
    backend.reset();
    window = nullptr;
    surfaceSize = {};
    resetRuntimeState();
}

bool MapView::renderFrame() {
    bool attemptedRender = false;
    bool rendered = false;
    auto renderIfNeeded = [&] {
        if (frontend && frontend->hasPendingRender()) {
            attemptedRender = true;
            if (frontend->renderFrame()) {
                ++renderedFrameCount;
                rendered = true;
            }
        }
    };

    // Keep interaction frames from waiting behind a burst of resource callbacks.
    renderIfNeeded();
    runLoopOnce();
    if (!attemptedRender) {
        renderIfNeeded();
    }

    return rendered;
}

void MapView::runLoopOnce() {
    util::RunLoop::Get()->runOnce();
}

void MapView::reduceMemoryUse() {
    if (frontend) {
        frontend->reduceMemoryUse();
    }
}

void MapView::setStyleURL(const std::string& url) {
    if (!map) {
        return;
    }

    resetRuntimeState();
    map->getStyle().loadURL(url);
}

void MapView::setStyleJSON(const std::string& json) {
    if (map) {
        resetRuntimeState();
        map->getStyle().loadJSON(json);
    }
}

void MapView::jumpTo(CameraOptions cameraOptions) {
    desiredCameraBounds.reset();
    desiredFreeCamera.reset();
    desiredCamera = mergeCameraOptions(cameraOptions);
    if (map) {
        map->jumpTo(std::move(cameraOptions));
    }
}

void MapView::easeTo(CameraOptions cameraOptions, AnimationOptions animationOptions) {
    desiredCameraBounds.reset();
    desiredFreeCamera.reset();
    desiredCamera = mergeCameraOptions(cameraOptions);
    if (map) {
        map->easeTo(std::move(cameraOptions), std::move(animationOptions));
    }
}

void MapView::flyTo(CameraOptions cameraOptions, AnimationOptions animationOptions) {
    desiredCameraBounds.reset();
    desiredFreeCamera.reset();
    desiredCamera = mergeCameraOptions(cameraOptions);
    if (map) {
        map->flyTo(std::move(cameraOptions), std::move(animationOptions));
    }
}

void MapView::fitBounds(CameraBoundsOptions options) {
    desiredFreeCamera.reset();
    desiredCameraBounds = options;
    if (map) {
        desiredCamera = cameraForBounds(options);
        map->jumpTo(*desiredCamera);
    }
}

void MapView::setFreeCameraOptions(FreeCameraOptions cameraOptions) {
    if (!cameraOptions.position && !cameraOptions.orientation) {
        return;
    }

    desiredCamera.reset();
    desiredCameraBounds.reset();
    if (map) {
        map->setFreeCameraOptions(cameraOptions);
        desiredFreeCamera = map->getFreeCameraOptions();
    } else {
        desiredFreeCamera = mergeFreeCameraOptions(cameraOptions);
    }
}

void MapView::setGestureInProgress(bool inProgress) {
    if (map) {
        map->setGestureInProgress(inProgress);
    }
}

void MapView::moveBy(double x, double y, AnimationOptions animationOptions) {
    if (!map) {
        return;
    }

    map->moveBy(logicalCoordinateForFramebufferCoordinate(x, y, pixelRatio), std::move(animationOptions));
    desiredCameraBounds.reset();
    desiredFreeCamera.reset();
    desiredCamera = map->getCameraOptions();
}

void MapView::pitchBy(double deltaPitch) {
    if (!map || !std::isfinite(deltaPitch)) {
        return;
    }

    const auto currentCamera = map->getCameraOptions();
    const auto bounds = map->getBounds();
    const double minPitch = bounds.minPitch.value_or(0.0);
    const double maxPitch = bounds.maxPitch.value_or(60.0);
    const double lowerPitch = std::min(minPitch, maxPitch);
    const double upperPitch = std::max(minPitch, maxPitch);
    const double currentPitch = currentCamera.pitch.value_or(0.0);
    const double nextPitch = std::clamp(currentPitch + deltaPitch, lowerPitch, upperPitch);
    map->easeTo(CameraOptions().withPitch(nextPitch), AnimationOptions{});
    desiredCameraBounds.reset();
    desiredFreeCamera.reset();
    desiredCamera = map->getCameraOptions();
}

void MapView::scaleBy(double scale, double anchorX, double anchorY) {
    if (!map || !std::isfinite(scale) || scale <= 0.0) {
        return;
    }

    map->scaleBy(scale, logicalCoordinateForFramebufferCoordinate(anchorX, anchorY, pixelRatio));
    desiredCameraBounds.reset();
    desiredFreeCamera.reset();
    desiredCamera = map->getCameraOptions();
}

void MapView::flyBy(double scale, double anchorX, double anchorY, AnimationOptions animationOptions) {
    if (!map || !std::isfinite(scale) || scale <= 0.0 || !std::isfinite(anchorX) || !std::isfinite(anchorY)) {
        return;
    }

    const auto currentCamera = map->getCameraOptions();
    const double currentZoom = currentCamera.zoom.value_or(0.0);
    const double nextZoom = currentZoom + std::log2(scale);
    map->flyTo(CameraOptions()
                   .withZoom(nextZoom)
                   .withAnchor(logicalCoordinateForFramebufferCoordinate(anchorX, anchorY, pixelRatio)),
               std::move(animationOptions));
    desiredCameraBounds.reset();
    desiredFreeCamera.reset();
    desiredCamera = map->getCameraOptions();
}

void MapView::rotateBy(double previousAngle, double currentAngle, double anchorX, double anchorY) {
    if (!map || !std::isfinite(previousAngle) || !std::isfinite(currentAngle) || !std::isfinite(anchorX) ||
        !std::isfinite(anchorY)) {
        return;
    }

    const auto currentCamera = map->getCameraOptions();
    const double currentBearing = currentCamera.bearing.value_or(0.0);
    const double bearingDelta = util::rad2deg(currentAngle - previousAngle);
    map->easeTo(CameraOptions()
                    .withBearing(currentBearing - bearingDelta)
                    .withAnchor(logicalCoordinateForFramebufferCoordinate(anchorX, anchorY, pixelRatio)),
                AnimationOptions{});
    desiredCameraBounds.reset();
    desiredFreeCamera.reset();
    desiredCamera = map->getCameraOptions();
}

CameraOptions MapView::getCameraOptions() const {
    return map ? map->getCameraOptions() : desiredCamera.value_or(CameraOptions());
}

std::vector<std::string> MapView::getStyleAttributions() const {
    if (!map) {
        return {};
    }

    std::set<std::string> seen;
    std::vector<std::string> attributions;
    for (const auto* source : map->getStyle().getSources()) {
        if (source == nullptr) {
            continue;
        }

        auto attribution = source->getAttribution();
        if (!attribution) {
            continue;
        }

        const auto first = attribution->find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            continue;
        }

        const auto last = attribution->find_last_not_of(" \t\r\n");
        auto trimmed = attribution->substr(first, last - first + 1);
        if (seen.insert(trimmed).second) {
            attributions.push_back(std::move(trimmed));
        }
    }
    return attributions;
}

FreeCameraOptions MapView::getFreeCameraOptions() const {
    return map ? map->getFreeCameraOptions() : desiredFreeCamera.value_or(FreeCameraOptions());
}

CameraOptions MapView::cameraForBounds(const CameraBoundsOptions& options) const {
    if (!map) {
        throw std::runtime_error("Camera for bounds requires an active map surface");
    }
    return map->cameraForLatLngBounds(options.bounds, options.padding, options.bearing, options.pitch);
}

void MapView::setDebugOptions(MapDebugOptions debugOptions_) {
    debugOptions = debugOptions_;
    if (map) {
        map->setDebug(debugOptions);
    }
}

void MapView::setBounds(BoundOptions boundOptions) {
    desiredBounds = mergeBoundOptions(boundOptions);
    if (map) {
        map->setBounds(std::move(boundOptions));
    }
}

BoundOptions MapView::getBounds() const {
    return map ? map->getBounds() : desiredBounds.value_or(BoundOptions());
}

bool MapView::hasPendingRender() const {
    return frontend && frontend->hasPendingRender();
}

bool MapView::isFullyLoaded() const {
    return map && map->isFullyLoaded();
}

std::int32_t MapView::getGlesContextClientVersion() const {
    return backend ? backend->getGlesContextClientVersion() : 0;
}

const std::string& MapView::getRendererDiagnostic() const {
    static const std::string empty;
    return backend ? backend->getRendererDiagnostic() : empty;
}

void MapView::setClientOptions(std::string name, std::string version) {
    if (clientName == name && clientVersion == version) {
        return;
    }

    clientName = std::move(name);
    clientVersion = std::move(version);
    if (window != nullptr && !surfaceSize.isEmpty()) {
        createMap(window, surfaceSize);
    }
}

void MapView::setResourceOptions(ResourceOptions options) {
    resourceOptions = std::move(options);
    if (window != nullptr && !surfaceSize.isEmpty()) {
        createMap(window, surfaceSize);
    }
}

void MapView::setTileCacheEnabled(bool enabled) {
    tileCacheEnabled = enabled;
    if (frontend) {
        frontend->setTileCacheEnabled(tileCacheEnabled);
    }
}

void MapView::setPixelRatio(float pixelRatio_) {
    if (!std::isfinite(pixelRatio_) || pixelRatio_ <= 0.0f) {
        throw std::invalid_argument("Pixel ratio must be a finite positive value");
    }

    if (pixelRatio == pixelRatio_) {
        return;
    }

    pixelRatio = pixelRatio_;
    if (window != nullptr && !surfaceSize.isEmpty()) {
        createMap(window, surfaceSize);
    }
}

void MapView::setSize(Size size) {
    surfaceSize = size;
    if (backend) {
        backend->setSize(size);
    }
    if (map) {
        map->setSize(logicalSizeForFramebufferSize(size, pixelRatio));
    }
}

void MapView::createMap(OHNativeWindow* newWindow, Size size) {
    clearSurface();

#if MLN_RENDER_BACKEND_VULKAN
    backend = std::make_unique<VulkanWindowBackend>(newWindow, size);
#else
    backend = std::make_unique<EGLWindowBackend>(newWindow, size);
#endif
    frontend = std::make_unique<RendererFrontend>(backend->getRendererBackend(), pixelRatio);
    frontend->setTileCacheEnabled(tileCacheEnabled);

    MapOptions mapOptions;
    mapOptions.withSize(logicalSizeForFramebufferSize(size, pixelRatio))
        .withPixelRatio(pixelRatio)
        .withMapMode(MapMode::Continuous);

    ClientOptions clientOptions;
    clientOptions.withName(clientName).withVersion(clientVersion);

    map = std::make_unique<Map>(*frontend, *this, mapOptions, resourceOptions, clientOptions);
    window = newWindow;
    surfaceSize = size;
    map->setDebug(debugOptions);

    applyDesiredBounds();
    applyDesiredCamera();
}

CameraOptions MapView::mergeCameraOptions(const CameraOptions& cameraOptions) const {
    CameraOptions merged = map ? map->getCameraOptions() : desiredCamera.value_or(CameraOptions());
    if (cameraOptions.center) {
        merged.center = cameraOptions.center;
    }
    if (cameraOptions.centerAltitude) {
        merged.centerAltitude = cameraOptions.centerAltitude;
    }
    if (cameraOptions.padding) {
        merged.padding = cameraOptions.padding;
    }
    if (cameraOptions.anchor) {
        merged.anchor = cameraOptions.anchor;
    }
    if (cameraOptions.zoom) {
        merged.zoom = cameraOptions.zoom;
    }
    if (cameraOptions.bearing) {
        merged.bearing = cameraOptions.bearing;
    }
    if (cameraOptions.pitch) {
        merged.pitch = cameraOptions.pitch;
    }
    if (cameraOptions.roll) {
        merged.roll = cameraOptions.roll;
    }
    if (cameraOptions.fov) {
        merged.fov = cameraOptions.fov;
    }
    return merged;
}

FreeCameraOptions MapView::mergeFreeCameraOptions(const FreeCameraOptions& cameraOptions) const {
    FreeCameraOptions merged = map ? map->getFreeCameraOptions() : desiredFreeCamera.value_or(FreeCameraOptions());
    if (cameraOptions.position) {
        merged.position = cameraOptions.position;
    }
    if (cameraOptions.orientation) {
        merged.orientation = cameraOptions.orientation;
    }
    return merged;
}

BoundOptions MapView::mergeBoundOptions(const BoundOptions& boundOptions) const {
    BoundOptions merged = getBounds();
    if (boundOptions.bounds) {
        merged.bounds = boundOptions.bounds;
    }
    if (boundOptions.minZoom) {
        merged.minZoom = boundOptions.minZoom;
    }
    if (boundOptions.maxZoom) {
        merged.maxZoom = boundOptions.maxZoom;
    }
    if (boundOptions.minPitch) {
        merged.minPitch = boundOptions.minPitch;
    }
    if (boundOptions.maxPitch) {
        merged.maxPitch = boundOptions.maxPitch;
    }
    return merged;
}

void MapView::applyDesiredBounds() {
    if (map && desiredBounds) {
        map->setBounds(*desiredBounds);
    }
}

void MapView::applyDesiredCamera() {
    if (!map) {
        return;
    }
    if (desiredFreeCamera) {
        map->setFreeCameraOptions(*desiredFreeCamera);
        return;
    }
    if (desiredCameraBounds) {
        desiredCamera = cameraForBounds(*desiredCameraBounds);
    }
    if (desiredCamera) {
        map->jumpTo(*desiredCamera);
    }
}

void MapView::onDidFailLoadingMap(MapLoadError, const std::string& message) {
    mapLoaded = false;
    styleLoaded = false;
    lastMapLoadError = message;
    Log::Error(Event::Style, lastMapLoadError);
}

void MapView::onDidFinishLoadingMap() {
    mapLoaded = true;
}

void MapView::onDidFinishLoadingStyle() {
    styleLoaded = true;
    if (!map) {
        return;
    }
    try {
        const auto defaultCamera = map->getStyle().getDefaultCamera();
        if (!desiredCamera && !desiredCameraBounds && !desiredFreeCamera && defaultCamera.center && defaultCamera.zoom) {
            desiredCameraBounds.reset();
            desiredCamera = defaultCamera;
            map->jumpTo(defaultCamera);
            return;
        }
    } catch (const std::exception& exception) {
        Log::Error(Event::Style, std::string("Style camera jump failed: ") + exception.what());
    } catch (...) {
        Log::Error(Event::Style, "Style camera jump failed");
    }
    applyDesiredCamera();
}

void MapView::onStyleImageMissing(const std::string& id) {
    lastStyleImageMissing = id;
}

void MapView::onGlyphsError(const FontStack&, const GlyphRange&, std::exception_ptr error) {
    lastGlyphsError = util::toString(error);
}

void MapView::onSpriteError(const std::optional<style::Sprite>&, std::exception_ptr error) {
    lastSpritesError = util::toString(error);
}

void MapView::onRenderError(std::exception_ptr error) {
    lastRenderError = util::toString(error);
    Log::Error(Event::Render, lastRenderError);
}

void MapView::resetRuntimeState() {
    styleLoaded = false;
    mapLoaded = false;
    renderedFrameCount = 0;
    lastMapLoadError.clear();
    lastRenderError.clear();
    lastStyleImageMissing.clear();
    lastGlyphsError.clear();
    lastSpritesError.clear();
}

} // namespace ohos
} // namespace mbgl
